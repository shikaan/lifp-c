#ifndef RESULT_H
#define RESULT_H

typedef enum {
    ERROR_UnexpectedToken,
} EXCEPTION_KIND;

typedef union {
    char unexpected_token;
} ExceptionPayload;

typedef struct {
    EXCEPTION_KIND kind;
    ExceptionPayload payload;
} Exception;

#define Result(T) \
	struct { \
		bool ok; \
		union { \
			T value; \
			Exception error; \
		}; \
	}

// Helper functions for creating results
#define ok(T, Value) (T){.ok = true, .value = (Value)}
#define err(T, Kind, Payload) (T){.ok = false, .error = (Exception){.kind = (Kind), .payload = Payload}}

#endif // RESULT_H
