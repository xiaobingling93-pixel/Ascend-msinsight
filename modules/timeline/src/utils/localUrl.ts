const urls = new URLSearchParams(window.location.search);

export const getSearchParams = (name: string): string | null => {
    return urls.get(name);
};
