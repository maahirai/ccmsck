#include "ccmsck.h"

//次のトークンが期待している記号の時にはトークンをひとつ読み進め，真を返す．そうでないときは偽を返す．
bool expect(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len
    ||memcmp(token->str,op,token->len))
    error_at(token->str,"'%s'ではありません",op);
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

//トークンが期待している記号のとき読み進め,真を返す．そうで無い場合は偽を返す.
bool consume(char *op){
  if(token->kind!=TK_RESERVED
    ||strlen(op)!=token->len||memcmp(token->str,op,token->len))
    return false;
  token=token->next;
  return true;
}

Token *consume_ident(){
  if(token->kind!=TK_IDENT)
    return NULL;
  Token *ret=token;
  token=token->next;
  return ret;
}

bool at_eof(){
  return token->kind==TK_EOF;
}

bool startswith(char *p,char *q){
  return memcmp(p,q,strlen(q))==0;
}

//現在着目しているトークン
Token *token;
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

    if(strchr("+-*/()<>;=",*p)){
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

    //variables
    if(*p>='a'&&*p<='z'){
      cur=new_token(TK_IDENT,cur,p,1);
      cur->len=1;
      cur->str=p;
      p++;
      continue;
    }

    error_at(p,"この文字は扱えません\n");
  }
  new_token(TK_EOF,cur,p,0);
  return head.next;
}

//ここから抽象構文木
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

Node *stmt(){
  Node *node=expr();
  expect(";");
  return node;
}

Node *expr(){
  return assign();
}

Node *assign(){
  Node *node=equality();

  if(consume("="))
    node=new_node(ND_ASSIGN,node,assign());
  return node;
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

  Token *tok=consume_ident();
  if(tok){
    Node *node=calloc(1,sizeof(Node));
    node->kind=ND_LVAR;
    node->offset=(tok->str[0]-'a'+1)*8;
    return node;
  }
  return new_node_num(expect_num());
}

Node *code[100];

void program(){
  int index=0;
  while(!at_eof())
    code[index++]=stmt();
  code[index]=NULL;
}