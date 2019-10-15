#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    TK_RESERVED,//記号
    TK_NUM,//整数トークン
    TK_EOF,//終端文字
} TokenKind;

typedef struct Token Token;

struct Token{
    TokenKind kind;//トークンの型
    Token *next;//次のトークンへのポインタ
    int value;//TK_NUMの場合の数値
    char *str;//入力文字列から生成されたトークン文字列
};

Token *token;

//エラーが起きたことを知らせる関数
//printfと同じ引数
char *user_input;
void error_at(char *loc, char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが引数と一致している場合はトークンを一つ読み進め，
//真を返す．そうでない場合偽を返す．
bool consume(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

//次のトークンが引数と一致している場合はトークンを一つ読み進める．
//そうでない場合はエラーを報告する．
void expect(char op){
    if(token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str,"%cではありません．",op);
    token=token->next;
}

//次のトークンが数値の場合はトークンを一つ読み進めてその値を返す．
//そうでない場合はエラーを報告する．
int expect_number(){
    if(token->kind != TK_NUM)
        error_at(token->str, "数値ではありません．");
    int value = token->value;
    token = token->next;
    return value;
}

bool at_eof(){
    return token->kind == TK_EOF;
}

//新しいトークンを生成してcurに繋げる.
Token *new_token(TokenKind kind, Token *cur, char *str){
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok ->str = str;
    cur -> next = tok;
    return tok;
}

//入力として与えられた文字列をトークンに切り分ける．(トークナイズ)
Token *tokenize(char *p){
    Token top;
    Token * cur =&top;
    top.next = NULL;

    while(*p){
        //空白文字を飛ばす．
        if(isspace(*p)){
            p++;
            continue;
        }

        if(*p == '+'||*p == '-'){
           cur = new_token(TK_RESERVED,cur,p++);
           continue;
        }

        if(isdigit(*p)){
            cur = new_token(TK_NUM,cur,p);
            cur->value = strtol(p,&p,10);
            continue;
        }

        error_at(p,"トークナイズできません.");
    }

    new_token(TK_EOF,cur,p);
    return top.next;
}




int main(int argc, char **argv) {
    user_input = argv[1];
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    //トークナイズする
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //式の最初は数でなければならないため，それをチェックして
    //最初のmov命令を出力
    printf("  mov rax, %d\n", expect_number());

    //終端文字を見つけるまで記号トークンと数値のセットを
    //取得し続け,それに対応したアセンブリを出力．
    while(!at_eof()){
        if(consume('+')){
            printf("  add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub rax, %d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}