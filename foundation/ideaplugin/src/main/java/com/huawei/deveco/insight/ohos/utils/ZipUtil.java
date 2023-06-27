/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 */

package com.huawei.deveco.insight.ohos.utils;

import org.apache.commons.io.FileUtils;
import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

/**
 * Zip utility class
 *
 * @since 2022-11-01
 */
public class ZipUtil {
    private static final LogPrinter LOGGER = LogPrinter.createLogger(ZipUtil.class);

    private static final int MAX_BUFFER_SIZE = 8196;

    private static final String DELIMITER = "/";

    private static final int MAX_UNZIP_SIZE = 2000000000;

    /**
     * Zip all files in srcDirPath into a .zip file without a folder
     *
     * @param srcDirPath src directory path
     * @param dstZipPath zip file path
     * @return true for success, false for failure
     */
    public static boolean zipAllFiles(String srcDirPath, String dstZipPath) {
        try (FileOutputStream fos = new FileOutputStream(dstZipPath); ZipOutputStream zos = new ZipOutputStream(fos)) {
            File fileDir = FileUtils.getFile(srcDirPath);
            File[] zipFiles = fileDir.listFiles();
            if (zipFiles == null) {
                LOGGER.warn("Failed to zipAllFiles, cannot list files in src path: {}.", srcDirPath);
                return false;
            }
            for (File file : zipFiles) {
                zos.putNextEntry(new ZipEntry(file.getName()));
                writeToFile(file, zos);
            }
            return true;
        } catch (FileNotFoundException exception) {
            LOGGER.warn("Failed to zipAllFiles, FileNotFoundException occurred.");
            return false;
        } catch (SecurityException exception) {
            LOGGER.warn("Failed to zipAllFiles, SecurityException occurred: {}.", exception.getMessage());
            return false;
        } catch (IOException exception) {
            LOGGER.warn("Failed to zipAllFiles, IOException occurred: {}.", exception.getMessage());
            return false;
        }
    }

    /**
     * Zip files into a folder named srcFile.getName() and then generate .zip file
     *
     * @param srcFilePath source file path
     * @param dstZipPath  destination zip path
     * @return true/false
     */
    public static boolean zipFolder(String srcFilePath, String dstZipPath) {
        try (FileOutputStream fos = new FileOutputStream(dstZipPath); ZipOutputStream zos = new ZipOutputStream(fos)) {
            File file = FileUtils.getFile(srcFilePath);
            try {
                zip(file, file.getName(), zos);
                return true;
            } catch (IOException exception) {
                LOGGER.warn("Failed to zip files, IOException occurred.");
                return false;
            }
        } catch (FileNotFoundException exception) {
            LOGGER.warn("Failed to zipFolder, FileNotFoundException occurred.");
            return false;
        } catch (SecurityException exception) {
            LOGGER.warn("Failed to zipFolder, SecurityException occurred: {}.", exception.getMessage());
            return false;
        } catch (IOException exception) {
            LOGGER.warn("Failed to zipFolder, IOException occurred: {}.", exception.getMessage());
            return false;
        }
    }

    /**
     * extract zip
     *
     * @param srcZipPath source zip path
     * @param dstDirPath destination directory path
     * @return true/false
     */
    public static boolean extractZip(String srcZipPath, String dstDirPath) {
        try (FileInputStream fis = new FileInputStream(srcZipPath); ZipInputStream zis = new ZipInputStream(fis)) {
            extract(zis, dstDirPath);
            return true;
        } catch (IOException ex) {
            LOGGER.warn("Failed to extract zip file {}", srcZipPath);
            return false;
        }
    }

    private static void zip(File srcFile, String fileName, ZipOutputStream dstZip) throws IOException {
        if (srcFile.isHidden()) {
            return;
        }
        if (srcFile.isDirectory()) {
            if (fileName.endsWith(DELIMITER)) {
                dstZip.putNextEntry(new ZipEntry(fileName));
            } else {
                dstZip.putNextEntry(new ZipEntry(fileName + DELIMITER));
            }
            dstZip.closeEntry();
            File[] children = srcFile.listFiles();
            if (children == null) {
                LOGGER.warn("Files list of dir {} is null", srcFile);
                return;
            }
            for (File childFile : children) {
                zip(childFile, fileName + DELIMITER + childFile.getName(), dstZip);
            }
        } else {
            dstZip.putNextEntry(new ZipEntry(fileName));
            writeToFile(srcFile, dstZip);
        }
    }

    private static void extract(ZipInputStream zis, String dstDir) throws IOException {
        ZipEntry entry = zis.getNextEntry();
        while (entry != null) {
            File file = getFile(dstDir, entry);
            if (entry.isDirectory()) {
                if (!file.isDirectory() && !file.mkdirs()) {
                    throw new IOException("Failed to create directory " + file);
                }
            } else {
                writeToZip(file, zis);
            }
            entry = zis.getNextEntry();
        }
        zis.closeEntry();
    }

    private static File getFile(String dstDir, ZipEntry zipEntry) throws IOException {
        if (zipEntry == null) {
            throw new IOException("The zipEntry is null in " + ZipUtil.class.getName());
        }
        File dstFile = FileUtils.getFile(sanitizeFileName(zipEntry.getName(), dstDir));
        String destDirPath = FileUtils.getFile(dstDir).getCanonicalPath();
        String destFilePath = dstFile.getCanonicalPath();
        if (!destFilePath.startsWith(destDirPath + File.separator)) {
            throw new IOException("The entry is outside of the target directory " + zipEntry.getName());
        }
        return dstFile;
    }

    private static String sanitizeFileName(String entryFileName, String intendedDir) throws IOException {
        File destFile = FileUtils.getFile(intendedDir, entryFileName);
        String destFilePath = destFile.getCanonicalPath();
        File intendedDirFile = FileUtils.getFile(intendedDir);
        String intendedDirPath = intendedDirFile.getCanonicalPath();
        if (destFilePath.startsWith(intendedDirPath)) {
            return destFilePath;
        } else {
            throw new IllegalStateException("File is outside extraction target directory,"
                    + "entryFileName is " + entryFileName);
        }
    }

    private static void writeToFile(File file, ZipOutputStream zipOutputStream) throws IOException {
        try (FileInputStream fis = new FileInputStream(file)) {
            byte[] bytes = new byte[MAX_BUFFER_SIZE];
            int length;
            while ((length = fis.read(bytes)) >= 0) {
                zipOutputStream.write(bytes, 0, length);
            }
        } catch (FileNotFoundException e) {
            LOGGER.warn("Failed to writeToFile, FileNotFoundException occurred.");
        } catch (SecurityException e) {
            LOGGER.warn("Failed to writeToFile, SecurityException occurred.");
        }
    }

    private static void writeToZip(@NotNull File file, @NotNull ZipInputStream zipInputStream) throws IOException {
        File parent = file.getParentFile();
        if (!parent.isDirectory() && !parent.mkdirs()) {
            throw new IOException("Failed to create directory " + parent);
        }
        // write file content
        try (FileOutputStream fos = new FileOutputStream(file)) {
            int total = 0;
            int length;
            byte[] buffer = new byte[MAX_BUFFER_SIZE];
            while ((length = zipInputStream.read(buffer)) > 0) {
                // check every entry's size
                total += length;
                if (total > MAX_UNZIP_SIZE) {
                    break;
                }
                fos.write(buffer, 0, length);
            }
        } catch (FileNotFoundException e) {
            LOGGER.warn("Failed to writeToZip, FileNotFoundException occurred.");
        } catch (SecurityException e) {
            LOGGER.warn("Failed to writeToZip, SecurityException occurred.");
        }
    }
}