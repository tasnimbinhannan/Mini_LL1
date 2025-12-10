#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAXSTACK 200
#define MAXTOK 500
#define MAXSYM 50

// ==========================================================
// TOKEN DEFINITIONS (for parser)
// ==========================================================
typedef enum
{
    T_HEADER,
    T_INT,
    T_DEC,
    T_RETURN,
    T_WHILE,
    T_BREAK,
    T_PRINTF,
    T_MAIN,
    T_ID_VAR,
    T_ID_FUNC,
    T_ID_LABEL,
    T_NUM,
    T_ASSIGN,
    T_PLUS,
    T_LT,
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_DOTDOT,
    T_COLON,
    T_EOF,
    T_INVALID
} TokenType;

typedef struct
{
    TokenType type;
    char lexeme[128];
} Token;

const char *token_type_to_string(TokenType t)
{
    switch (t)
    {
    case T_HEADER:
        return "HEADER";
    case T_INT:
        return "INT";
    case T_DEC:
        return "DEC";
    case T_RETURN:
        return "RETURN";
    case T_WHILE:
        return "WHILE";
    case T_BREAK:
        return "BREAK";
    case T_PRINTF:
        return "PRINTF";
    case T_MAIN:
        return "MAIN";
    case T_ID_VAR:
        return "ID_VAR";
    case T_ID_FUNC:
        return "ID_FUNC";
    case T_ID_LABEL:
        return "ID_LABEL";
    case T_NUM:
        return "NUM";
    case T_ASSIGN:
        return "ASSIGN";
    case T_PLUS:
        return "PLUS";
    case T_LT:
        return "LT";
    case T_LPAREN:
        return "LPAREN";
    case T_RPAREN:
        return "RPAREN";
    case T_LBRACE:
        return "LBRACE";
    case T_RBRACE:
        return "RBRACE";
    case T_DOTDOT:
        return "DOTDOT";
    case T_COLON:
        return "COLON";
    case T_EOF:
        return "$";
    default:
        return "INVALID";
    }
}

// ==========================================================
// LEXER: 77‑STATE DFA (from labtask13 (2).c) :contentReference[oaicite:1]{index=1}
// ==========================================================

#define NUM_STATES 77
#define NUM_INPUTS 28
#define MAX_TOKEN_LEN 100

// State Enum (exactly as lab file)
enum
{
    D0,
    D_I,
    D_IN,
    D_INT, // int
    D_D,
    D_DE,
    D_DEC, // dec
    D_R,
    D_RE,
    D_RET,
    D_RETU,
    D_RETUR,
    D_RETURN, // return
    D_B,
    D_BR,
    D_BRE,
    D_BREA,
    D_BREAK, // break
    D_W,
    D_WH,
    D_WHI,
    D_WHIL,
    D_WHILE, // while
    D_P,
    D_PR,
    D_PRI,
    D_PRIN,
    D_PRINT,
    D_PRINTF, // printf
    D_M,
    D_MA,
    D_MAI,
    D_MAIN, // main
    D_L,
    D_LO,
    D_LOO,
    D_LOOP,
    D_LOOP_U,
    D_LOOP_VAR,
    D_LOOP_D1,
    D_LOOP_D2,
    D_LOOP_COLON, // loop_...
    D_VAR_S,
    D_VAR_A,
    D_VAR_D,
    D_VAR_FINAL, // variable
    D_F_ID_0,
    D_F_F,
    D_F_FN, // function ending in Fn
    D_DOL,
    D_ASSIGN, // $=
    D_DOT,
    D_ST_END, // ..
    D_DIG,
    D_DIG_DOT,
    D_DIG_DEC, // numbers
    // Filler states (just to get total 77)
    D_EXT_1,
    D_EXT_2,
    D_EXT_3,
    D_EXT_4,
    D_EXT_5,
    D_EXT_6,
    D_EXT_7,
    D_EXT_8,
    D_EXT_9,
    D_EXT_10,
    D_EXT_11,
    D_EXT_12,
    D_EXT_13,
    D_EXT_14,
    D_EXT_15,
    DEAD
};

// Lexer display names for each accepting state
const char *accepting_tokens[NUM_STATES];
// Parser token mapping for each accepting state
TokenType accepting_types[NUM_STATES];

// Input Mapping (same idea as lab)
int get_input(char c)
{
    if (c == '_')
        return 0;
    if (c == 'd')
        return 1;
    if (c == 'e')
        return 2;
    if (c == 'c')
        return 3;
    if (c == 'i')
        return 4;
    if (c == 'n')
        return 5;
    if (c == 't')
        return 6;
    if (c == 'r')
        return 7;
    if (c == 'u')
        return 8;
    if (c == 'b')
        return 9;
    if (c == 'a')
        return 10;
    if (c == 'k')
        return 11;
    if (c == 'w')
        return 12;
    if (c == 'h')
        return 13;
    if (c == 'l')
        return 14;
    if (c == 'o')
        return 15;
    if (c == 'p')
        return 16;
    if (c == 'f')
        return 17;
    if (c == 'm')
        return 18;
    if (c == 's')
        return 19;
    if (isdigit((unsigned char)c))
        return 20;
    if (c == '.')
        return 21;
    if (c == '$')
        return 22;
    if (c == '=')
        return 23;
    if (c == ':')
        return 24;
    if (c == 'F')
        return 25;
    if (isalpha((unsigned char)c))
        return 26;
    return 27;
}

// Nicely formatted lexical output
void print_formatted(const char *lexeme, const char *token_type_str)
{
    printf("%-20s -> %s\n", lexeme, token_type_str);
}

// -------------------------------
// LEXER: tokenize hello.c -> tokens[]
// -------------------------------
int lex_file(const char *filename, Token tokens[], int maxTokens)
{

    // 1) init display + parser-token map
    for (int i = 0; i < NUM_STATES; i++)
    {
        accepting_tokens[i] = "";
        accepting_types[i] = T_INVALID;
    }

    // keywords / reserved (display name, parser token)
    accepting_tokens[D_INT] = "INT";
    accepting_types[D_INT] = T_INT;

    accepting_tokens[D_DEC] = "DEC";
    accepting_types[D_DEC] = T_DEC;

    accepting_tokens[D_RETURN] = "RETURN";
    accepting_types[D_RETURN] = T_RETURN;

    accepting_tokens[D_BREAK] = "BREAK";
    accepting_types[D_BREAK] = T_BREAK;

    accepting_tokens[D_WHILE] = "WHILE";
    accepting_types[D_WHILE] = T_WHILE;

    accepting_tokens[D_PRINTF] = "PRINTF";
    accepting_types[D_PRINTF] = T_PRINTF;

    accepting_tokens[D_MAIN] = "MAIN";
    accepting_types[D_MAIN] = T_MAIN;

    // loop label – we accept at state D_LOOP_D2 (just before colon)
    accepting_tokens[D_LOOP_D2] = "ID_LABEL";
    accepting_types[D_LOOP_D2] = T_ID_LABEL;

    // variable
    accepting_tokens[D_VAR_FINAL] = "ID_VAR";
    accepting_types[D_VAR_FINAL] = T_ID_VAR;

    // function ID ending in Fn
    accepting_tokens[D_F_FN] = "ID_FUNC";
    accepting_types[D_F_FN] = T_ID_FUNC;

    // assignment =
    accepting_tokens[D_ASSIGN] = "ASSIGN";
    accepting_types[D_ASSIGN] = T_ASSIGN;

    // statement terminator ..
    accepting_tokens[D_ST_END] = "DOTDOT";
    accepting_types[D_ST_END] = T_DOTDOT;

    // numbers (both int / dec treated as NUM)
    accepting_tokens[D_DIG] = "NUM";
    accepting_types[D_DIG] = T_NUM;

    accepting_tokens[D_DIG_DEC] = "NUM";
    accepting_types[D_DIG_DEC] = T_NUM;

    // 2) DFA transition table (exactly from labtask13 (2).c, minor tweak on label accept)
    int next_state[NUM_STATES][NUM_INPUTS] = {
        // Row D0
        {D_VAR_S, D_D, D_F_ID_0, D_F_ID_0, D_I, D_F_ID_0, D_F_ID_0, D_R, D_F_ID_0, D_B, D_F_ID_0, D_F_ID_0, D_W, D_F_ID_0, D_L, D_F_ID_0, D_P, D_F_ID_0, D_M, D_F_ID_0, D_DIG, D_DOT, D_DOL, D_ASSIGN, D_F_ID_0, D_F_ID_0, D_F_ID_0, DEAD},
        // D_I, D_IN, D_INT
        {DEAD, DEAD, DEAD, DEAD, DEAD, D_IN, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_INT, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // D_D, D_DE, D_DEC
        {DEAD, DEAD, D_DE, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, D_DEC, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // return group
        {DEAD, DEAD, D_RE, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_RET, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_RETU, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_RETUR, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, D_RETURN, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // break
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_BR, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, D_BRE, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_BREA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_BREAK, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // while
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_WH, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, D_WHI, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_WHIL, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, D_WHILE, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // printf
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_PR, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, D_PRI, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, D_PRIN, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_PRINT, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_PRINTF, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // main
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_MA, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, D_MAI, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, D_MAIN, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // loop_
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_LO, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_LOO, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_LOOP, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {D_LOOP_U, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, DEAD, DEAD, DEAD, DEAD, D_LOOP_VAR, D_LOOP_VAR, DEAD},
        {DEAD, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_VAR, D_LOOP_D1, DEAD, DEAD, DEAD, DEAD, D_LOOP_VAR, D_LOOP_VAR, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_LOOP_D2, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_LOOP_COLON, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // variable DFA
        {DEAD, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, DEAD, DEAD, DEAD, DEAD, DEAD, D_VAR_A, D_VAR_A, DEAD},
        {DEAD, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_A, D_VAR_D, DEAD, DEAD, DEAD, DEAD, D_VAR_A, D_VAR_A, DEAD},
        {DEAD, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, D_VAR_FINAL, DEAD, DEAD, DEAD, DEAD, DEAD, D_VAR_FINAL, D_VAR_FINAL, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // function‑ID (…Fn)
        {DEAD, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, DEAD, DEAD, DEAD, DEAD, DEAD, D_F_F, D_F_ID_0, DEAD},
        {DEAD, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_FN, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, DEAD, DEAD, DEAD, DEAD, DEAD, D_F_F, D_F_ID_0, DEAD},
        {DEAD, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, D_F_ID_0, DEAD, DEAD, DEAD, DEAD, DEAD, D_F_F, D_F_ID_0, DEAD},
        // '='
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_ASSIGN, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // '.' '..'
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_ST_END, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // numbers
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_DIG, D_DIG_DOT, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_DIG_DEC, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, D_DIG_DEC, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        // filler rows (all DEAD)
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD},
        {DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD, DEAD}};

    // number DFA tweak (like in lab)
    next_state[D_DIG][21] = DEAD; // single '.' after number not allowed unless going via D_DIG_DOT

    // open file
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        printf("Error: cannot open %s\n", filename);
        return -1;
    }

    int tokIndex = 0;

    printf("===== LEXICAL ANALYSIS =====\n");

    // header line
    char hdr[256];
    if (fgets(hdr, sizeof(hdr), fp))
    {
        if (strstr(hdr, "#include<stdio.h>"))
        {
            tokens[tokIndex].type = T_HEADER;
            strcpy(tokens[tokIndex].lexeme, "#include<stdio.h>");
            tokIndex++;
            print_formatted("#include<stdio.h>", "HEADER");
        }
        else
        {
            printf("Lexical Error: First line must be #include<stdio.h>\n");
            fclose(fp);
            return -1;
        }
    }
    else
    {
        printf("Lexical Error: Empty file\n");
        fclose(fp);
        return -1;
    }

    int current_state = D0;
    char current_lexeme[MAX_TOKEN_LEN];
    int lex_len = 0;
    current_lexeme[0] = '\0';

    int c;
    while ((c = fgetc(fp)) != EOF && tokIndex < maxTokens - 1)
    {

        // comment style //...  (optional, like lab)
        if (c == '/')
        {
            int d = fgetc(fp);
            if (d == '/')
            {
                // finalize existing token (if any)
                if (current_state != D0 && accepting_types[current_state] != T_INVALID)
                {
                    current_lexeme[lex_len] = '\0';
                    print_formatted(current_lexeme, accepting_tokens[current_state]);
                    tokens[tokIndex].type = accepting_types[current_state];
                    strcpy(tokens[tokIndex].lexeme, current_lexeme);
                    tokIndex++;
                }
                current_state = D0;
                lex_len = 0;
                current_lexeme[0] = '\0';

                while ((c = fgetc(fp)) != EOF && c != '\n')
                    ; // skip rest of line
                continue;
            }
            else
            {
                if (d != EOF)
                    ungetc(d, fp);
            }
        }

        // whitespace => finalize token
        if (isspace(c))
        {
            if (current_state != D0 && accepting_types[current_state] != T_INVALID)
            {
                current_lexeme[lex_len] = '\0';
                print_formatted(current_lexeme, accepting_tokens[current_state]);
                tokens[tokIndex].type = accepting_types[current_state];
                strcpy(tokens[tokIndex].lexeme, current_lexeme);
                tokIndex++;
            }
            current_state = D0;
            lex_len = 0;
            current_lexeme[0] = '\0';
            continue;
        }

        // colon: if we are inside label/token, first finish previous, then colon token
        if (c == ':' && current_state != D0)
        {
            if (accepting_types[current_state] != T_INVALID)
            {
                current_lexeme[lex_len] = '\0';
                print_formatted(current_lexeme, accepting_tokens[current_state]);
                tokens[tokIndex].type = accepting_types[current_state];
                strcpy(tokens[tokIndex].lexeme, current_lexeme);
                tokIndex++;
            }
            else
            {
                current_lexeme[lex_len] = '\0';
                printf("%-20s -> Rejected by DFA\n", current_lexeme);
                fclose(fp);
                return -1;
            }
            current_state = D0;
            lex_len = 0;
            current_lexeme[0] = '\0';

            char t[2] = {':', '\0'};
            print_formatted(t, "COLON");
            tokens[tokIndex].type = T_COLON;
            strcpy(tokens[tokIndex].lexeme, t);
            tokIndex++;
            continue;
        }

        // single‑char tokens when at start (like lab)
        if (current_state == D0 && strchr("(){}<+,", c))
        {
            char t[2] = {c, '\0'};
            if (c == '(')
            {
                print_formatted(t, "LPAREN");
                tokens[tokIndex].type = T_LPAREN;
            }
            else if (c == ')')
            {
                print_formatted(t, "RPAREN");
                tokens[tokIndex].type = T_RPAREN;
            }
            else if (c == '{')
            {
                print_formatted(t, "LBRACE");
                tokens[tokIndex].type = T_LBRACE;
            }
            else if (c == '}')
            {
                print_formatted(t, "RBRACE");
                tokens[tokIndex].type = T_RBRACE;
            }
            else if (c == '<')
            {
                print_formatted(t, "LT");
                tokens[tokIndex].type = T_LT;
            }
            else if (c == '+')
            {
                print_formatted(t, "PLUS");
                tokens[tokIndex].type = T_PLUS;
            }
            else
            { // comma – parser actually doesn't use it
                print_formatted(t, "COMMA");
                // we won't add COMMA token for parser (not needed in grammar)
                current_state = D0;
                continue;
            }
            strcpy(tokens[tokIndex].lexeme, t);
            tokIndex++;
            continue;
        }

        // normal DFA step
        int in = get_input((char)c);
        int next = next_state[current_state][in];

        if (next == DEAD)
        {
            // try finalize previous token greedily
            if (current_state != D0)
            {
                ungetc(c, fp); // push back char

                if (accepting_types[current_state] != T_INVALID)
                {
                    current_lexeme[lex_len] = '\0';
                    print_formatted(current_lexeme, accepting_tokens[current_state]);
                    tokens[tokIndex].type = accepting_types[current_state];
                    strcpy(tokens[tokIndex].lexeme, current_lexeme);
                    tokIndex++;
                }
                else
                {
                    current_lexeme[lex_len] = '\0';
                    printf("%-20s -> Rejected by DFA\n", current_lexeme);
                    fclose(fp);
                    return -1;
                }
                current_state = D0;
                lex_len = 0;
                current_lexeme[0] = '\0';
            }
            else
            {
                // invalid char at fresh start (skip or treat as error)
                printf("Lexical Error: unexpected char '%c'\n", c);
                fclose(fp);
                return -1;
            }
        }
        else
        {
            // extend current lexeme
            if (lex_len < MAX_TOKEN_LEN - 1)
            {
                current_lexeme[lex_len++] = (char)c;
                current_lexeme[lex_len] = '\0';
            }
            current_state = next;
        }
    }

    // finalize EOF pending token
    if (current_state != D0 && accepting_types[current_state] != T_INVALID)
    {
        current_lexeme[lex_len] = '\0';
        print_formatted(current_lexeme, accepting_tokens[current_state]);
        tokens[tokIndex].type = accepting_types[current_state];
        strcpy(tokens[tokIndex].lexeme, current_lexeme);
        tokIndex++;
    }

    fclose(fp);

    // append EOF token
    tokens[tokIndex].type = T_EOF;
    strcpy(tokens[tokIndex].lexeme, "$");
    tokIndex++;

    return tokIndex;
}

// =======================================================
// LL(1) PARSER (same LL(1) style as lab code) :contentReference[oaicite:2]{index=2}
// =======================================================

// Nonterminals
char *NT[] = {
    "Program", "FuncList", "Func", "Type", "FuncName", "ParamOpt", "Param",
    "StmtList", "Stmt", "Loop", "LoopBody", "Expr", "ExprPrime", "Term", "Factor", "RetVal"};
#define NNT 16

// Terminals (names must match token_type_to_string)
char *TERMINALS[] = {
    "HEADER", "INT", "DEC", "RETURN", "WHILE", "BREAK", "PRINTF", "MAIN",
    "ID_VAR", "ID_FUNC", "ID_LABEL", "NUM", "ASSIGN", "PLUS", "LT", "LPAREN",
    "RPAREN", "LBRACE", "RBRACE", "DOTDOT", "COLON", "$"};
#define NTER 22

// Right‑hand sides
char *RHS[] = {
    "HEADER FuncList",                                                                     // 1
    "Func FuncList",                                                                       // 2
    "",                                                                                    // 3 epsilon
    "Type FuncName LPAREN ParamOpt RPAREN LBRACE StmtList RBRACE",                         // 4
    "INT",                                                                                 // 5
    "DEC",                                                                                 // 6
    "MAIN",                                                                                // 7
    "ID_FUNC",                                                                             // 8
    "Param",                                                                               // 9
    "",                                                                                    // 10 epsilon
    "Type ID_VAR",                                                                         // 11
    "Stmt StmtList",                                                                       // 12
    "",                                                                                    // 13 epsilon
    "Type ID_VAR ASSIGN Expr DOTDOT",                                                      // 14
    "PRINTF LPAREN ID_VAR RPAREN DOTDOT",                                                  // 15
    "Loop",                                                                                // 16
    "RETURN RetVal DOTDOT",                                                                // 17
    "BREAK DOTDOT",                                                                        // 18
    "ID_LABEL COLON WHILE LPAREN Type ID_VAR LT NUM DOTDOT RPAREN LBRACE LoopBody RBRACE", // 19
    "StmtList",                                                                            // 20
    "Term ExprPrime",                                                                      // 21
    "PLUS Term ExprPrime",                                                                 // 22
    "",                                                                                    // 23 epsilon
    "Factor",                                                                              // 24
    "ID_VAR",                                                                              // 25
    "NUM",                                                                                 // 26
    "ID_FUNC LPAREN ID_VAR RPAREN",                                                        // 27
    "ID_VAR",                                                                              // 28
    "NUM"                                                                                  // 29
};

// LL(1) parsing table
int TABLE[NNT][NTER] = {
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3},
    {0, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 7, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 0, 0, 0, 0, 0},
    {0, 11, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 12, 12, 12, 0, 12, 12, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 13, 0, 0, 0},
    {0, 14, 14, 17, 0, 18, 15, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 20, 20, 20, 0, 20, 20, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 0, 0, 23, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 24, 24, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 25, 27, 0, 26, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 28, 0, 0, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

// parser stack
char stackArr[MAXSTACK][MAXSYM];
int top = -1;

void push(const char *s)
{
    if (top < MAXSTACK - 1)
    {
        strcpy(stackArr[++top], s);
    }
}

char *popStack()
{
    if (top >= 0)
        return stackArr[top--];
    return NULL;
}

void print_stack()
{
    printf("[");
    for (int i = top; i >= 0; i--)
    {
        printf("%s", stackArr[i]);
        if (i > 0)
            printf(", ");
    }
    printf("]");
}

int find_nt(const char *x)
{
    for (int i = 0; i < NNT; i++)
        if (strcmp(NT[i], x) == 0)
            return i;
    return -1;
}

int find_t(const char *x)
{
    for (int i = 0; i < NTER; i++)
        if (strcmp(TERMINALS[i], x) == 0)
            return i;
    return -1;
}

// =======================================================
// main: run lexer (DFA) → parser (LL(1))
// =======================================================
int main()
{
    Token tokens[MAXTOK];

    int tokenCount = lex_file("check.c", tokens, MAXTOK);
    if (tokenCount <= 0)
    {
        printf("\nLexical analysis failed.\n");
        return 1;
    }

    // build token string array for LL(1) parser
    char input[MAXTOK][MAXSYM];
    int n = 0;
    for (int i = 0; i < tokenCount; i++)
    {
        const char *name = token_type_to_string(tokens[i].type);
        strcpy(input[n++], name);
    }
    input[n][0] = '\0';

    // init parser stack
    top = -1;
    push("$");
    push(NT[0]); // Program

    int ip = 0;

    printf("\n===== PARSING (LL(1) table driven) =====\n");
    printf("%-40s %-15s %-15s %-30s\n",
           "Stack", "Lookahead", "Top", "Production Applied");
    printf("------------------------------------------------------------------------------------------\n");

    while (top >= 0)
    {
        char X[MAXSYM];
        strcpy(X, popStack());
        char *a = input[ip];

        printf("%-40s %-15s %-15s ", "", a, X);

        int tindex = find_t(X);

        if (tindex != -1)
        { // terminal
            if (strcmp(X, a) == 0)
            {
                printf("%-30s\n", "match");
                ip++;
            }
            else
            {
                printf("REJECTED (terminal mismatch)\n");
                return 0;
            }
            print_stack();
            printf("\n");
            continue;
        }

        int ntindex = find_nt(X);
        int aindex = find_t(a);

        if (ntindex == -1 || aindex == -1)
        {
            printf("REJECTED (unknown symbol)\n");
            return 0;
        }

        int prod = TABLE[ntindex][aindex];

        if (prod == 0)
        {
            printf("REJECTED (no rule for (%s,%s))\n", X, a);
            return 0;
        }

        if (strlen(RHS[prod - 1]) == 0)
            printf("%-30s\n", "epsilon");
        else
            printf("%-30s\n", RHS[prod - 1]);

        if (strlen(RHS[prod - 1]) > 0)
        {
            char temp[200];
            strcpy(temp, RHS[prod - 1]);
            char *p = strtok(temp, " ");
            char symbols[20][MAXSYM];
            int k = 0;
            while (p)
            {
                strcpy(symbols[k++], p);
                p = strtok(NULL, " ");
            }
            for (int i = k - 1; i >= 0; i--)
                push(symbols[i]);
        }
        print_stack();
        printf("\n");
    }

    if (strcmp(input[ip], "") == 0)
    {
        printf("\nFinal verdict: Program ACCEPTED.\n");
    }
    else
    {
        printf("\nFinal verdict: Program REJECTED (input not fully consumed).\n");
    }

    return 0;
}
