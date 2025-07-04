#include "lexer.h"

ResultTokenList tokenize([[maybe_unused]] const char *source) {
    // Token *list = nullptr;
    // return ok(ResultTokenList, list);
    return err(ResultTokenList, ERROR_UnexpectedToken, { .unexpected_token = 'c' });
}
