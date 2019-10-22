#include"ccmsck.h"

Token *token;
char *user_input;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
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