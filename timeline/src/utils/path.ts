export function fileNameOfPath(path: string): string {
    return path.substring(1 + path.lastIndexOf('/'));
}
