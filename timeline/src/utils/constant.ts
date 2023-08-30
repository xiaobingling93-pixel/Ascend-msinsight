export type Phase = 'configuring' | 'waiting' | 'recording' | 'analyzing' | 'download' | 'error';

export const stateTexts: Record<Phase, string> = {
    waiting: 'Initializing...',
    recording: 'Recording...',
    analyzing: 'Analyzing...',
    configuring: '',
    download: '',
    error: '',
};
