/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 */

package com.huawei.ascend.insight;

import static com.huawei.ascend.insight.common.constant.Constant.INSIGHT_NOTIFY;

import com.huawei.ascend.insight.common.EventKey;
import com.huawei.ascend.insight.common.ProjectContext;
import com.huawei.ascend.insight.common.constant.URLConstants;
import com.huawei.ascend.insight.handlers.SelectFolderHandler;
import com.huawei.ascend.insight.model.dto.JcefRequest;
import com.huawei.ascend.insight.resourcehandler.InsightRequestHandler;
import com.huawei.ascend.insight.service.ServerHelper;
import com.huawei.ascend.insight.ui.MessagePanel;
import com.huawei.ascend.insight.utils.CefMessageRouterProxy;
import com.huawei.ascend.insight.utils.JsonUtil;
import com.huawei.ascend.insight.utils.LogPrinter;
import com.huawei.ascend.insight.utils.LogProperties;

import com.intellij.DynamicBundle;
import com.intellij.notification.NotificationGroupManager;
import com.intellij.notification.NotificationType;
import com.intellij.openapi.project.Project;
import com.intellij.openapi.util.Disposer;
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
import org.cef.callback.CefQueryCallback;
import org.cef.handler.CefContextMenuHandler;
import org.cef.handler.CefContextMenuHandlerAdapter;
import org.cef.handler.CefLifeSpanHandler;
import org.cef.handler.CefLifeSpanHandlerAdapter;
import org.cef.handler.CefMessageRouterHandlerAdapter;
import org.jetbrains.annotations.NotNull;

import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeListener;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
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

    private static final Map<String, CefQueryHandler> CEF_QUERY_HANDLER_MAP = Collections.unmodifiableMap(
            new HashMap<>() {
                {
                    put(EventKey.INSIGHT_IMPORT, new SelectFolderHandler());
                }
            });

    private static ToolWindow toolWindow;

    private static volatile int insightCount = 0;

    private static Project project = null;

    private static JBCefBrowser webView;

    static {
        hookThread();
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
            ServerHelper.cancelServerHook();
            ServerHelper.destroy();
            CefMessageRouterProxy.getInstance().removeRouter(webView.getCefBrowser());
            // 移除webView注册的相关内容
            cefBrowser.getClient().removeMessageRouter(router);
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
            ServerHelper.cancelServerHook();
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
                        NotificationGroupManager.getInstance()
                            .getNotificationGroup(INSIGHT_NOTIFY)
                            .createNotification(DynamicBundle.message(bundle, "singleInsightTip"),
                                NotificationType.INFORMATION)
                            .notify(project);
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
        ServerHelper.startServer();
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
        webView = JBCefBrowser.createBuilder().setOffScreenRendering(false).build();
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

        String homePage =
            Boolean.getBoolean(USE_DEBUG_HOME_PAGE_KEY) ? URLConstants.DEBUG_HOME_PAGE : URLConstants.HOME_PAGE;
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
            Optional.ofNullable(webView)
                .map(JBCefBrowserBase::getCefBrowser)
                .ifPresent(cefBrowser -> cefBrowser.executeJavaScript(command, cefBrowser.getURL(), 0));
        };
        component.addPropertyChangeListener("foreground", propertyChangeListener);
    }

    private void addMessageRouter(JBCefBrowser webView) {
        router = CefMessageRouter.create();
        MessageRouterHandlerAdapter cefMessageRouterHandler = new MessageRouterHandlerAdapter();
        CefMessageRouterProxy routerHandlerProxy = CefMessageRouterProxy.getInstance();
        routerHandlerProxy.putCefRouter(webView.getCefBrowser(), cefMessageRouterHandler);
        router.addHandler(routerHandlerProxy, true);
        webView.getCefBrowser().getClient().addMessageRouter(router);
    }

    private static void hookThread() {
        // hook the Runtime thread
        Runtime.getRuntime().addShutdownHook(new WindowShutdownHook("HookThread insight"));
    }

    public static Project getProject() {
        return project;
    }

    static class MessageRouterHandlerAdapter extends CefMessageRouterHandlerAdapter {
        @Override
        public boolean onQuery(CefBrowser browser, CefFrame frame, long queryId, String request, boolean isPersistent,
                               CefQueryCallback callback) {
            try {
                JcefRequest jcefRequest = JsonUtil.parseObject(request, JcefRequest.class);
                CefQueryHandler handler = CEF_QUERY_HANDLER_MAP.get(jcefRequest.getKey());
                handler.onQuery(jcefRequest.getData().getMethod(), jcefRequest.getData().getParams(), callback);
            } catch (Exception e) {
                LOGGER.warn("Failed to invoke handler, cause {}", e.getMessage());
            }
            return true;
        }
    }
}
