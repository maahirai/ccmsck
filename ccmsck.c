#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<stdarg.h>
#include<stdbool.h>
#include<string.h>

//トークンの種類
typedef enum{
  TK_RESERVED,//記号
  TK_NUM,     //整数トークン
  TK_EOF      //入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token
{
  TokenKind kind; //トークンの型
  Token *next;  //次の入力トークン
  int val;     //kindがTK_NUMの場合，その数値
  char *str;   //トークン文字列
};

//現在着目しているトークン
Token *token;

//トークンが期待している記号のとき読み進め,真を返す．そうで無い場合は偽を返す.
bool consume(char op){
  if(token->kind!=TK_RESERVED||token->str[0]!=op)
    return false;
  token=token->next;
  return true;
}

//エラーを報告するための関数
//可変長引数
void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr,  fmt,  ap);
  fprintf(stderr, "\n");
  exit(1);
}

//入力されたプログラム
char *user_input;

void error_at(char *loc,char *fmt, ... ){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n",user_input);
  fprintf(stderr, "%*s",pos," ");//pos個の空白を出力
  fprintf(stderr,"^ ");
  vfprintf(stderr,fmt, ap);
  fprintf(stderr,"\n");
  exit(1);
}

//次のトークンが期待している記号の時にはトークンをひとつ読み進め，真を返す．そうでないときは偽を返す．
bool expect(char op){
  if(token->kind!=TK_RESERVED||token->str[0]!=op)
    error(token->str,"'%c'ではありません",op);
  token=token->next;
}

//次のトークンが数値の場合，トークンを一つ読み進めてその数値を返す．そうで無い場合にはエラーを報告する．
int expect_num(){
  if(token->kind!=TK_NUM)
    error_at(token->str,"数値ではありません");
  int ret=token->val;
  token=token->next;
  return ret;
}

bool at_eof(){
  return token->kind==TK_EOF;
}

//新しいトークンを作成してcurに繋げる．
Token *new_token(TokenKind kind,Token *cur,char *str){
  Token *tok=calloc(1,sizeof(Token));
  cur->next=tok;
  tok->str=str;
  tok->kind=kind;
  return tok;
}

Token *tokenize(){
  char *p=user_input;
  Token head;
  head.next=NULL;
  Token *cur=&head;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p=='+'||*p=='-'){
      cur=new_token(TK_RESERVED,cur,p++);
      continue;
    }

    if(isdigit(*p)){
      cur=new_token(TK_NUM,cur,p);
      cur->val=strtol(p,&p,10);
      continue;
    }

    error_at(p,"この文字は扱えません\n");
  }
  new_token(TK_EOF,cur,p);
  return head.next;
}


int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }

  user_input=argv[1];
  token=tokenize();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  printf("  mov rax, %d\n",expect_num());

  while(!at_eof()){
      if(consume('+')){
        printf("  add rax, %d\n",expect_num());
        continue;
      }
      if(consume('-')){
        printf("  sub rax, %d\n",expect_num());
        continue;
      }
      error("処理不可能");
  }
  printf("  ret\n");
  return 0;
}