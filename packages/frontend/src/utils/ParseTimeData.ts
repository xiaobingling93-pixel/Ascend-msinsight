export class FormatDate {
    formatTimeData = (): string => {
        const Dates = new Date();
        const Year: string = Dates.getFullYear().toString();
        const Months: string = (Dates.getMonth() + 1) < 10 ? '0' + (Dates.getMonth() + 1).toString() : (Dates.getMonth() + 1).toString();
        const Day: string = Dates.getDate() < 10 ? '0' + Dates.getDate().toString() : Dates.getDate().toString();
        const Hours: string = Dates.getHours() < 10 ? '0' + Dates.getHours().toString() : Dates.getHours().toString();
        const Minutes: string = Dates.getMinutes() < 10 ? '0' + Dates.getMinutes().toString() : Dates.getMinutes().toString();
        const Seconds: string = Dates.getSeconds() < 10 ? '0' + Dates.getSeconds().toString() : Dates.getSeconds().toString();
        return Year + '-' + Months + '-' + Day + '-' + Hours + ':' + Minutes + ':' + Seconds;
    };
}
