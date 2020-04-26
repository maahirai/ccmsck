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
  int len;     //トークンの長さ
};

//現在着目しているトークン
Token *token;


//トークンが期待している記号のとき読み進め,真を返す．そうで無い場合は偽を返す.
bool consume(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len||
    memcmp(token->str,op,token->len))
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
bool expect(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len||memcmp(token->str,op,token->len))
    error(token->str,"'%s'ではありません",op);
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

bool startswith(char *p,char *q){
  return memcmp(p,q,strlen(q))==0;
}

//新しいトークンを作成してcurに繋げる．
Token *new_token(TokenKind kind,Token *cur,char *str,int len){
  Token *tok=calloc(1,sizeof(Token));
  cur->next=tok;
  tok->str=str;
  tok->kind=kind;
  tok->len=len;
  return tok;
}

Token *tokenize(char *p){
  Token head;
  head.next=NULL;
  Token *cur=&head;

  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }
    if(startswith(p,"==")||startswith(p,"!=")||
      startswith(p,">=")||startswith(p,"<=")){
        cur = new_token(TK_RESERVED,cur,p,2);
        p+=2;
        continue;
      }

    if(strchr("+-*/()<>",*p)){
      cur=new_token(TK_RESERVED,cur,p++,1);
      continue;
    }

    if(isdigit(*p)){
      cur=new_token(TK_NUM,cur,p,0);
      char *q=p;
      cur->val=strtol(p,&p,10);
      cur->len=p-q;
      continue;
    }

    error_at(p,"この文字は扱えません\n");
  }
  new_token(TK_EOF,cur,p,0);
  return head.next;
}

//ここから抽象構文木
typedef enum{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node {
  NodeKind kind;
  Node *lhs;  //右辺
  Node *rhs;  //左辺
  int val;    //ND_NUMのとき使う
};

Node *new_node(NodeKind kind, Node *lhs,Node *rhs){
  Node *node=(Node *)calloc(1,sizeof(Node));
  node->kind=kind;
  node->lhs=lhs;
  node->rhs=rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node=(Node *)calloc(1,sizeof(Node));
  node->kind=ND_NUM;
  node->val=val;
  return node;
}

//プロトタイプ宣言
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Node *expr(){
  return equality();
}

Node *equality(){
  Node *node=relational();

  for(; ;){
    if(consume("==")){
      node=new_node(ND_EQ,node,relational());
    }
    else if(consume("!=")){
      node=new_node(ND_NE,node,relational());
    }
    else
      return node;
  }
}

Node *relational(){
  Node *node=add();

  for(; ;){
    if(consume("<")){
      node=new_node(ND_LT,node,add());
    }
    else if(consume("<=")){
      node=new_node(ND_LE,node,add());
    }
    else if(consume(">")){
      node=new_node(ND_LT,add(),node);
    }
    else if(consume(">=")){
      node=new_node(ND_LE,add(),node);
    }
    else
      return node;
  }
}

Node *add(){
  Node *node=mul();

  for(; ;){
    if(consume("+")){
      node=new_node(ND_ADD,node,mul());
    }
    else if(consume("-")){
      node=new_node(ND_SUB,node,mul());
    }
    else
      return node;
  }
}

Node *mul(){
  Node *node = unary();

  for(;;){
    if(consume("*")){
      node = new_node(ND_MUL, node ,unary());
    }
    else if(consume("/")){
      node = new_node(ND_DIV, node ,unary());
    }
    else
      return node;
  }
}

Node *unary(){
  if(consume("+")){
    return unary();
  }
  else if(consume("-")){
    return new_node(ND_SUB,new_node_num(0),unary());
  }
  return primary();
}

Node *primary(){
  if(consume("(")){
    Node *node = expr();
    expect(")"); //対応するカッコとじ
    return node;
  }
  return new_node_num(expect_num());
}

void gen(Node *node){
  if(node->kind==ND_NUM){
    printf("  push %d\n",node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind)
  {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax,  rdi\n");
    printf("  sete al\n");
    printf("  movzb rax,  al\n");
    break;
  case ND_NE:
    printf("  cmp rax,  rdi\n");
    printf("  setne al\n");
    printf("  movzb rax,  al\n");
    break;
  case ND_LT:
    printf("  cmp rax,  rdi\n");
    printf("  setl al\n");
    printf("  movzb rax,  al\n");
    break;
  case ND_LE:
    printf("  cmp rax,  rdi\n");
    printf("  setle al\n");
    printf("  movzb rax,  al\n");
    break;
  default:
    error("不正なトークン\n");
  }
  printf("  push rax\n");
}

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