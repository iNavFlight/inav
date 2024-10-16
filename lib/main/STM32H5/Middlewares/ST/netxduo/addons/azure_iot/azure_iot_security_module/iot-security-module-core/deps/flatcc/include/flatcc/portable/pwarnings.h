#ifndef PWARNINGS_H
#define PWARNINGS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * See also pdiagnostics.h headers for per file control of common
 * warnings.
 *
 * This file is intended for global disabling of warnings that shouldn't
 * be present in C11 or perhaps C99, or a generally just noise where
 * recent clang / gcc compile cleanly with high warning levels.
 */

#if defined(_MSC_VER)
/* Needed when flagging code in or out and more. */
#pragma warning(disable: 4127) /* conditional expression is constant */
/* happens also in MS's own headers. */
#pragma warning(disable: 4668) /* preprocessor name not defined */
/* MSVC does not respect double parenthesis for intent */
#pragma warning(disable: 4706) /* assignment within conditional expression */
/* `inline` only advisory anyway. */
#pragma warning(disable: 4710) /* function not inlined */
/* Well, we don't intend to add the padding manually. */
#pragma warning(disable: 4820) /* x bytes padding added in struct */

/*
 * Don't warn that fopen etc. are unsafe
 *
 * Define a compiler flag like `-D_CRT_SECURE_NO_WARNINGS` in the build.
 * For some reason it doesn't work when defined here.
 *
 *     #define _CRT_SECURE_NO_WARNINGS
 */

/*
 * Anonymous union in struct is valid in C11 and has been supported in
 * GCC and Clang for a while, but it is not C99. MSVC also handles it,
 * but warns. Truly portable code should perhaps not use this feature,
 * but this is not the place to complain about it.
 */
#pragma warning(disable: 4201) /* nonstandard extension used: nameless struct/union */

#endif /* _MSV_VER */

#ifdef __cplusplus
}
#endif

#endif /* PWARNINGS_H */
