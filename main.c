#include"ccmsck.h"

//入力されたプログラム
char *user_input;

int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

  user_input=argv[1];
  token=tokenize(user_input);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  gen(node);

  //スタックに残った値が答えなのでraxにロードして，返り値とする．
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}