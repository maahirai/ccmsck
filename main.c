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
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  //プロローグ
  //変数26個分の領域の確保
  printf("  push  rbp\n");
  printf("  mov rbp,  rsp\n");
  printf("  sub rsp,  208\n");

  int index=0;
  while(code[index]){
    gen(code[index++]);

    //スタックに残った値が答えなのでraxにロードして，返り値とする．
    printf("  pop rax\n");
  }

  // エピローグ
  //　最後の式の結果がraxに残っているのでそれが返り値になる.
  printf("  mov rsp,  rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}