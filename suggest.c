#include <stdio.h>
#include <string.h>

// Suggestion with line number and expected keywords
void suggestKeyword(const char *word, const char *expectedKeywords[], int numKeywords, int line) {
    for (int k = 0; k < numKeywords; ++k) {
        const char *target = expectedKeywords[k];
        if (strlen(word) == strlen(target)) {
            int diff = 0;
            for (int i = 0; i < (int)strlen(target); ++i)
                if (word[i] != target[i]) diff++;
            if (diff == 1 || diff == 2) {
                printf("Did you mean '%s'? (line %d)\n", target, line);
                return;
            }
        }
    }
}