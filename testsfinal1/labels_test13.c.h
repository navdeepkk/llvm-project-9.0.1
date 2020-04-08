struct s_main{
int* n;
};
struct s_main_fun1{
struct s_main *s;
};
struct s_main_fun1_fun2{
struct s_main_fun1 *s;
};

void main_fun1(struct s_main *par_s);
void main_fun1_fun2(struct s_main_fun1 *par_s);
