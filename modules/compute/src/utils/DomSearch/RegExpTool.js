/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
export default class RegExpTool {
    #defaultOptions = {
        diacritics: true,
        synonyms: {},
        accuracy: 'partially',
        caseSensitive: false,
        ignoreJoiners: false,
        ignorePunctuation: [],
        wildcards: 'disabled',
    };

    constructor(options) {
        this.options = { ...this.#defaultOptions, ...options };
    }

    get options() {
        return this._options;
    }

    set options(value) {
        this._options = { ...this.#defaultOptions, ...value };
    }

    createRegExp(pattern) {
        let processed = pattern;
        const processingSteps = [
            this.#handleWildcardsInitial,
            this.#escapeSpecialCharacters,
            this.#processSynonyms,
            this.#handleJoinersAndPunctuation,
            this.#mergeWhitespace,
            this.#finalizeJoiners,
            this.#finalizeWildcards,
            this.#applyAccuracyRules,
        ];

        processingSteps.forEach(step => {
            processed = step.call(this, processed);
        });

        return processed;
    }

    // 私有方法使用#前缀
    #handleWildcardsInitial = (str) => {
        if (this.options.wildcards === 'disabled') {
            return str;
        }
        return str
            .replace(/(?<!\\)\?/g, '\u0001')
            .replace(/(?<!\\)\*/g, '\u0002');
    };

    #escapeSpecialCharacters = (str) => {
        return str.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
    };

    #processSynonyms = (str) => {
        const { synonyms, caseSensitive } = this.options;
        if (!Object.keys(synonyms).length) {
            return str;
        }

        const flags = `gm${caseSensitive ? '' : 'i'}`;
        const joiner = this.#shouldAddJoiners() ? '\u0000' : '';

        return Object.entries(synonyms).reduce((acc, [key, value]) => {
            const escapedKey = this.#escapeSynonym(key);
            const escapedValue = this.#escapeSynonym(value);
            const pattern = `(${escapedKey}|${escapedValue})`;

            return acc.replace(
                new RegExp(pattern, flags),
                `${joiner}(${this.#processSynonymPart(escapedKey)}|${this.#processSynonymPart(escapedValue)})${joiner}`,
            );
        }, str);
    };

    #shouldAddJoiners = () => {
        return this.options.ignoreJoiners || this.options.ignorePunctuation.length > 0;
    };

    #escapeSynonym = (term) => {
        return this.options.wildcards !== 'disabled'
            ? this.#handleWildcardsInitial(term)
            : this.#escapeSpecialCharacters(term);
    };

    #processSynonymPart = (term) => {
        return this.#shouldAddJoiners()
            ? this.#setupIgnoreJoiners(term)
            : term;
    };

    #setupIgnoreJoiners = (str) => {
        return str.replace(/([^(|)\\])/g, (match, p1) => {
            const nextChar = str[str.indexOf(p1) + 1];
            return /[(|)\\]/.test(nextChar) ? p1 : `${p1}\u0000`;
        });
    };

    #handleJoinersAndPunctuation = (str) => {
        return this.#shouldAddJoiners() ? this.#setupIgnoreJoiners(str) : str;
    };

    #mergeWhitespace = (str) => {
        return str.replace(/\s+/g, '[\\s]+');
    };

    #finalizeJoiners = (str) => {
        const joiners = [
            ...(this.options.ignorePunctuation || []),
            ...(this.options.ignoreJoiners ? ['\u00ad\u200b\u200c\u200d'] : []),
        ].join('');

        return joiners.length > 0
            // eslint-disable-next-line no-control-regex
            ? str.split(/\u0000+/).join(`[${joiners}]*`)
            : str;
    };

    #finalizeWildcards = (str) => {
        if (this.options.wildcards === 'disabled') {
            return str;
        }

        const wildcardMap = {
            '\u0001': this.options.wildcards === 'withSpaces' ? '[\\S\\s]?' : '\\S?',
            '\u0002': this.options.wildcards === 'withSpaces' ? '[\\S\\s]*?' : '\\S*?',
        };

        // eslint-disable-next-line no-control-regex
        return str.replace(/[\u0001\u0002]/g, m => wildcardMap[m]);
    };

    #applyAccuracyRules = (str) => {
        const { accuracy } = this.options;
        const limiters = typeof accuracy === 'object' ? accuracy.limiters : [];
        const limiterPattern = this.#createLimiterPattern(limiters);

        switch (accuracy.value || accuracy) {
            case 'partially':
            case 'complementary':
                return `()([^${limiterPattern}]*${str}[^${limiterPattern}]*)`;
            case 'exactly':
                return `(^|\\s${limiterPattern})(${str})(?=$|\\s${limiterPattern})`;
            default:
                return `()(${str})`;
        }
    };

    #createLimiterPattern = (limiters) => {
        const specialChars = '!\\"#$%&\'()*+,-./:;<=>?@[\\]^_`{|}~¡¿';
        const customLimiters = limiters.map(l => this.#escapeSpecialCharacters(l)).join('');
        return `\\s${customLimiters}${specialChars}`;
    };
}
