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

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // 整数
} NodeKind;

typedef struct Token Token;
struct Token{
    TokenKind kind;//トークンの型
    Token *next;//次のトークンへのポインタ
    int value;//TK_NUMの場合の数値
    char *str;//入力文字列から生成されたトークン文字列
};

typedef struct Node Node;
struct Node {
    NodeKind kind; //ノードの型
    Node *lhs;  //左辺に来るノード
    Node *rhs;  //右辺に来るノード
    int value; //kindがND_NUMの場合の数値
};

//プロトタイプ宣言
Node* expr();
Node* mul();
Node* primary();
Node* unary();

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

        if(strchr("+-*/()",*p)){
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

//抽象構文木のノードを生成します．(数値以外)
Node *new_node(NodeKind kind,Node *lhs,Node *rhs){
    Node *node = calloc(1,sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数値のノードを生成し,トークンを読み進めます．
Node *new_node_num(int value){
    Node *node_num = calloc(1,sizeof(Node));
    node_num->kind = ND_NUM;
    node_num->value = value;
    return node_num;
}

Node* expr(){
    Node *node = mul();


    for(; ;){
        if(consume('+')){
            node = new_node(ND_ADD,node,mul());
        }
        else if(consume('-')){
            node = new_node(ND_SUB,node,mul());
        }
        else
            return node;
    }
}

Node* mul(){
    Node *node = unary();

    for(; ;){
        if(consume('*')){
            node = new_node(ND_MUL,node,unary());
        }
        else if(consume('/')){
            node = new_node(ND_DIV,node,unary());
        }
        else
            return node;
    }
}
Node* unary(){
    if(consume('+')){
        return primary();
    }
    if(consume('-')){
        return new_node(ND_SUB,new_node_num(0),primary());
    }
    return primary();
}

Node *primary(){
    if(consume('(')){
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

void gen(Node *node){
    if(node->kind == ND_NUM){
        printf("    push %d\n",node->value);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    //元バージョンはswitch文
    if(node->kind == ND_ADD){
        printf("    add rax, rdi\n");
    }
    else if(node->kind == ND_SUB){
        printf("    sub rax, rdi\n");
    }
    else if(node->kind == ND_MUL){
        printf("    imul rax, rdi\n");
    }
    else if(node->kind == ND_DIV){
        printf("    cqo\n");
        printf("    idiv rdi\n");
    }

    printf("    push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    user_input = argv[1];
    //トークナイズする
    token = tokenize(user_input);
    //パースして抽象構文木を生成する．
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    //抽象構文木からアセンブリを生成する.
    gen(node);

    //スタックトップの値を取り出して返り値とする.
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}