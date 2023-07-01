/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package com.huawei.ascend.insight;

import static com.huawei.ascend.insight.common.constant.Constant.INSIGHT_NOTIFY;

import com.huawei.ascend.insight.common.ProjectContext;
import com.huawei.ascend.insight.common.constant.CmdConstants;
import com.huawei.ascend.insight.common.constant.URLConstants;
import com.huawei.ascend.insight.resourcehandler.InsightRequestHandler;
import com.huawei.ascend.insight.ui.MessagePanel;
import com.huawei.ascend.insight.utils.*;

import com.intellij.DynamicBundle;
import com.intellij.notification.NotificationGroupManager;
import com.intellij.notification.NotificationType;
import com.intellij.openapi.application.PathManager;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.Disposer;
import com.intellij.openapi.util.SystemInfo;
import com.intellij.openapi.wm.ToolWindow;
import com.intellij.openapi.wm.ToolWindowFactory;
import com.intellij.ui.content.Content;
import com.intellij.ui.content.ContentFactory;
import com.intellij.ui.jcef.JBCefApp;
import com.intellij.ui.jcef.JBCefBrowser;
import com.intellij.ui.jcef.JBCefBrowserBase;
import com.intellij.util.ui.UIUtil;

import org.cef.browser.CefBrowser;
import org.cef.browser.CefFrame;
import org.cef.browser.CefMessageRouter;
import org.cef.callback.CefContextMenuParams;
import org.cef.callback.CefMenuModel;
import org.cef.callback.CefNativeAdapter;
import org.cef.handler.CefContextMenuHandler;
import org.cef.handler.CefContextMenuHandlerAdapter;
import org.cef.handler.CefLifeSpanHandler;
import org.cef.handler.CefLifeSpanHandlerAdapter;
import org.jetbrains.annotations.NotNull;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.ResourceBundle;

import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * init Insight plugin when click Insight plugin
 *
 * @since 2022-04-02
 */
public class InsightWindowFactory implements ToolWindowFactory {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(InsightWindowFactory.class);

    private static final String USE_DEBUG_HOME_PAGE_KEY = "insight.useDebugHomepage";

    private static final CefContextMenuHandler NO_MENU = new CefContextMenuHandlerAdapter() {
        @Override
        public void onBeforeContextMenu(CefBrowser browser, CefFrame frame, CefContextMenuParams params,
            CefMenuModel model) {
            if (model != null) {
                model.clear();
            }
        }
    };

    private static ToolWindow toolWindow;

    private static volatile int insightCount = 0;

    private static Project project = null;

    private static boolean isCloseIDE = false;

    private static JBCefBrowser webView;

    static {
        hookThread();
        ProcessUtils.killProcess(CmdConstants.DIC_SERVER);
    }

    private PropertyChangeListener propertyChangeListener;

    private final InsightRequestHandler insightRequestHandler = new InsightRequestHandler();

    private final CefLifeSpanHandler lifeSpanHandler = new CefLifeSpanHandlerAdapter() {
        @Override
        public void onBeforeClose(CefBrowser cefBrowser) {
            synchronized (InsightWindowFactory.class) {
                insightCount--;
                onBeforeCloseStopSession(cefBrowser);
                LOGGER.info("closed, insightCount:{}", insightCount);
            }
        }

        // 关闭窗口之前发送相关命令暂停session
        private void onBeforeCloseStopSession(CefBrowser cefBrowser) {
            ProcessUtils.killProcess(CmdConstants.DIC_SERVER);
            // 移除webView注册的相关内容
            cefBrowser.getClient().removeMessageRouter(router);
            isCloseIDE = true;
            webView.getJBCefClient().removeRequestHandler(insightRequestHandler, cefBrowser);
            webView.getJBCefClient().removeLifeSpanHandler(lifeSpanHandler, cefBrowser);
            webView.getComponent().removePropertyChangeListener("foreground", propertyChangeListener);
        }
    };

    private CefMessageRouter router;

    private static class WindowShutdownHook extends Thread {
        /**
         * WindowShutdownHook
         *
         * @param name name
         */
        public WindowShutdownHook(@NotNull String name) {
            super(name);
        }

        @Override
        public void run() {
        }
    }

    @Override
    public void createToolWindowContent(@NotNull Project project, @NotNull ToolWindow toolWindow) {
        LOGGER.info("insightCount begin:{}", insightCount);
        if (insightCount < 1) {
            initInsightWindow(project, toolWindow);
            LOGGER.info("start with ui, insightCount:{}", insightCount);
            return;
        }
        synchronized (InsightWindowFactory.class) {
            LOGGER.info("start with empty, insightCount:{}", insightCount);
            ResourceBundle bundle;
            if ("zh".equals(DynamicBundle.getLocale().getLanguage())) {
                bundle = ResourceBundle.getBundle("message.zh");
            } else {
                bundle = ResourceBundle.getBundle("message.en");
            }
            MessagePanel mPanel = new MessagePanel();
            JPanel messagePanel = mPanel.getRootPanel();
            JLabel messageLabel = mPanel.getMessageLabel();
            messageLabel.setText(DynamicBundle.message(bundle, "singleInsightTip"));

            ContentFactory contentFactory = ContentFactory.getInstance();
            Content content = contentFactory.createContent(messagePanel, "", false);

            messageLabel.addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    LOGGER.warn("message panel clicked! current insight count: {}", insightCount);
                    if (insightCount < 1) {
                        toolWindow.getContentManager().removeContent(content, true);
                        initInsightWindow(project, toolWindow);
                    } else {
                        NotificationGroupManager.getInstance().getNotificationGroup(INSIGHT_NOTIFY)
                                .createNotification(DynamicBundle.message(bundle, "singleInsightTip"),
                                        NotificationType.INFORMATION).notify(project);
                    }
                }
            });

            toolWindow.getContentManager().addContent(content);
        }
    }

    // 调用此方法前 已进行一次insightCount < 1判断 调用后加锁再次判断 类似于双重检查锁
    private void initInsightWindow(@NotNull Project project, @NotNull ToolWindow toolWindow) {
        synchronized (InsightWindowFactory.class) {
            if (insightCount >= 1) {
                return;
            }
            insightCount++;
            LOGGER.info("insightCount end:{}", insightCount);
        }
        InsightWindowFactory.project = project;
        ProjectContext.setProject(project);
        if (!JBCefApp.isSupported()) {
            throw new IllegalStateException("JCEF is not supported");
        }
        if (startServer()) {
            LOGGER.info("start profiler server success");
        } else {
            LOGGER.info("start profiler server failed");
            BalloonNotification.show("Fail to start profiler server", NotificationType.WARNING);
        }
//        initSocket();
        initJCEFBrowser(project, toolWindow);
    }


    /**
     * initJCEFBrowser
     *
     * @param project project
     * @param tw tw
     */
    private void initJCEFBrowser(Project project, ToolWindow tw) {
        toolWindow = tw;
        LOGGER.info("insightCount: start initJCEFBrowser");
        ProjectContext.setProject(project);
        LOGGER.info("insightCount: start createBrowser");
        webView = JBCefBrowser.createBuilder()
                .setOffScreenRendering(true)
                .build();
        webView.getJBCefClient().addRequestHandler(insightRequestHandler, webView.getCefBrowser());
        webView.getJBCefClient().addLifeSpanHandler(lifeSpanHandler, webView.getCefBrowser());

        // clean contextMenu
        boolean isDebugMode = LogProperties.getLogLevel().getLevel() >= LogProperties.LogLevel.DEBUG.getLevel();
        if (!isDebugMode) {
            webView.getJBCefClient().addContextMenuHandler(NO_MENU, webView.getCefBrowser());
        }

        addMessageRouter(webView);
        Disposer.register(project, webView);

        // add webview to tool window
        ContentFactory contentFactory = ContentFactory.getInstance();
        Content content = contentFactory.createContent(webView.getComponent(), "", false);
        toolWindow.getContentManager().addContent(content);
        addPropertyChangeListener(webView);

        String homePage = Boolean.getBoolean(USE_DEBUG_HOME_PAGE_KEY)
                ? URLConstants.DEBUG_HOME_PAGE
                : URLConstants.HOME_PAGE;
        String url = homePage + "?language=" + DynamicBundle.getLocale().getLanguage();
        webView.loadURL(url);
        if (isDebugMode) {
            webView.openDevtools();
        }
        boolean isCreated = false;
        CefNativeAdapter adapter = null;
        CefBrowser cefBrowser = webView.getCefBrowser();
        if (cefBrowser instanceof CefNativeAdapter) {
            adapter = (CefNativeAdapter) cefBrowser;
        }
        if (adapter != null) {
            isCreated = adapter.getNativeRef("CefBrowser") != 0;
        }
        LOGGER.info("isCefBrowserCreated: {}", isCreated);
        LOGGER.info("insightCount: end loadURL:{}", url);
    }

    private void addPropertyChangeListener(JBCefBrowser webView) {

        JComponent component = webView.getComponent();
        propertyChangeListener = event -> {
            String command = "window.setTheme(" + UIUtil.isUnderDarcula() + ");";
            Optional.ofNullable(webView).map(JBCefBrowserBase::getCefBrowser)
                    .ifPresent(cefBrowser -> cefBrowser.executeJavaScript(command, cefBrowser.getURL(), 0));
        };
        component.addPropertyChangeListener("foreground", propertyChangeListener);
    }

    private void addMessageRouter(JBCefBrowser webView) {
        router = CefMessageRouter.create();
        webView.getCefBrowser().getClient().addMessageRouter(router);
    }

    private static void hookThread() {
        // hook the Runtime thread
        Runtime.getRuntime().addShutdownHook(new WindowShutdownHook("HookThread insight"));
    }

    public static Project getProject() {
        return project;
    }


    private boolean startServer() {
        if (ProcessUtils.findProcess(CmdConstants.DIC_SERVER)) {
            return true;
        }
        String pluginsPath = PathManager.getPluginsPath() + "\\ascnend-insight\\tools";
        List<String> processArgs = new ArrayList<>();
        if (SystemInfo.isWindows) {
            processArgs.add(CmdConstants.WINDOWS_CMD);
            processArgs.add(CmdConstants.WINDOWS_CMD_TERMINAL);
            processArgs.add(CmdConstants.DIC_SERVER);
        } else if (SystemInfo.isLinux) {
            processArgs.add("chmod");
            processArgs.add("+x");
            processArgs.add(CmdConstants.DIC_SERVER);
            processArgs.add("&&");
            processArgs.add("./" + CmdConstants.DIC_SERVER);
        } else {
            LOGGER.info("start dicServer error, system not supported");
        }
        Optional<Process> process = ProcessUtils.execute(processArgs, pluginsPath);
        return process.isPresent();
    }

    private void initSocket() {
        ThreadUtil.runInUIThread(new Runnable() {
            @Override public void run() {
                while (!isCloseIDE) {
                    try {
                        ServerSocket serverSocket = new ServerSocket(3001);
                        Socket socket = serverSocket.accept();
                        InputStream in = socket.getInputStream();
                        //从Socket中得到网络输出流，将数据发送到网络上
                        OutputStream out = socket.getOutputStream();

                        //接收客户端发来的数据
                        byte[] bs = new byte[1024];
                        //将数据存入bs数组中，返回值为数组的长度
                        int len = in.read(bs);
                        String str = new String(bs, 0, len, StandardCharsets.UTF_8);
                        System.out.println("来自客户端的消息: " + str);

                        //向客户端写数据,注意客户端代码中别忘记写read()方法,否则会抛异常
                        out.write("欢迎访问，你好，我是服务端".getBytes());
                        System.out.println("服务端正常结束");
                        socket.close();
                    } catch (IOException e) {
                        throw new RuntimeException(e);
                    }

                }
            }
        });
    }
}
