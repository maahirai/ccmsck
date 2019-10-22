#include"ccmsck.h"

//次のトークンが引数と一致している場合はトークンを一つ読み進め，
//真を返す．そうでない場合偽を返す．
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}
//次のトークンが引数と一致している場合はトークンを一つ読み進める．
//そうでない場合はエラーを報告する．
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "%sではありません．", op);
    token = token->next;
}
//次のトークンが数値の場合はトークンを一つ読み進めてその値を返す．
//そうでない場合はエラーを報告する．
int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "数値ではありません．");
    int value = token->value;
    token = token->next;
    return value;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

//新しいトークンを生成してcurに繋げる.
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

//入力として与えられた文字列をトークンに切り分ける．(トークナイズ)
Token *tokenize(char *p)
{
    Token top;
    Token *cur = &top;
    top.next = NULL;

    while (*p)
    {
        //空白文字を飛ばす．
        if (isspace(*p))
        {
            p++;
            continue;
        }

        //まず２文字から成る記号か調べる．
        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        //1文字の記号.
        if (strchr("+-*/()<>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *temp = p;
            cur->value = strtol(p, &p, 10);
            cur->len = p - temp;
            continue;
        }

        error_at(p, "トークナイズできません.");
    }

    new_token(TK_EOF, cur, p, 0);
    return top.next;
}

//ノードの生成
Node *new_node(NodeKind kind){
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
}

//抽象構文木にノードをを組み込む．(数値以外)
Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

//数値のノードを生成する．
Node *new_num(int value)
{
    Node *node = new_node(ND_NUM);
    node->value = value;
    return node;
}

// expr = equality
Node *expr()
{
    return equality();
}

// equality = relational("==" relational | "!=" relational)*
Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume("=="))
        {
            node = new_binary(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_binary(ND_NE, node, relational());
        }
        else
            return node;
    }
}

// relational = add("<=" add | "<" add | ">=" add | ">" add)*
Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume("<="))
        {
            node = new_binary(ND_LE, node, add());
        }
        else if (consume("<"))
        {
            node = new_binary(ND_LT, node, add());
        }
        else if (consume(">="))
        {
            node = new_binary(ND_LE, add(), node);
        }
        else if (consume(">"))
        {
            node = new_binary(ND_LT, add(), node);
        }
        else
            return node;
    }
}

//add = mul("+" mul | "-" mul)*
Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume("+"))
        {
            node = new_binary(ND_ADD, node, mul());
        }
        else if (consume("-"))
        {
            node = new_binary(ND_SUB, node, mul());
        }
        else
            return node;
    }
}

//mul = unary ("*" unary | "/" unary)*
Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume("*"))
        {
            node = new_binary(ND_MUL, node, unary());
        }
        else if (consume("/"))
        {
            node = new_binary(ND_DIV, node, unary());
        }
        else
            return node;
    }
}

// unary = ("+"|"-")? primary
Node *unary()
{
    if (consume("+"))
    {
        return unary();
    }
    if (consume("-"))
    {
        return new_binary(ND_SUB, new_num(0), unary());
    }
    return primary();
}

// primary = num | "(" expr ")"
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }

    return new_num(expect_number());
}