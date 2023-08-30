export const notificationTestHandler = (data: { info: string }): void => {
    console.log('Received server notification', data.info);
};
