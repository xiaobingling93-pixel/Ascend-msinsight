import fs from 'fs';

/**
 * 获取card id，即文件中rank_id属性
 *
 * @param filePath 文件路径
 */
export function parseCardID(filePath: string): string {
    let rankId = '';
    try {
        const fd = fs.openSync(filePath, 'r');
        const buf = Buffer.alloc(128);
        fs.readSync(fd, buf, 0, 128, 0);
        const bufString = buf.toString('utf-8');
        console.log('bufString: ', bufString);
        const start = bufString.indexOf('{');
        const end = bufString.indexOf('}');
        const rankIDJsonString = bufString.substring(start, end + 1);
        const rank = JSON.parse(rankIDJsonString);
        rankId = rank.rank_id;
    } catch (err) {
        console.log(err);
    }
    return rankId;
}
