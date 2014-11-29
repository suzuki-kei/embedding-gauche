#include <stdio.h>
#include <gauche.h>

/**
 * ScmEvalPacket や ScmLoadPacket に含まれる例外情報を表示する.
 */
#define PRINT_EXCEPTION(packet) \
    printf("[%s] %s\n", \
           SCM_STRING_CONST_CSTRING(Scm_ConditionTypeName(packet.exception)), \
           SCM_STRING_CONST_CSTRING(Scm_ConditionMessage(packet.exception)));

/**
 * オブジェクトの内容を表示する.
 * Scheme における (write object) 相当の処理を行う.
 */
void print_object(ScmObj object)
{
    ScmObj output = Scm_MakeOutputStringPort(TRUE);
    Scm_Write(object, output, SCM_WRITE_DISPLAY);
    printf("%s\n", SCM_STRING_CONST_CSTRING(Scm_GetOutputString(SCM_PORT(output), 0)));
    Scm_ClosePort(SCM_PORT(output));
}

void sample_basic()
{
    ScmObj value;

    value = SCM_MAKE_BOOL(1);
    printf("BOOLP: %s\n", SCM_BOOLP(value) ? "true" : "false");
    printf("BOOL_VALUE: %d\n", SCM_BOOL_VALUE(value));
    printf("\n");

    value = SCM_MAKE_INT(123);
    printf("INTP: %s\n", SCM_INTP(value) ? "true" : "false");
    printf("INT_VALUE: %ld\n", SCM_INT_VALUE(value));
    printf("\n");

    value = SCM_MAKE_CHAR('A');
    printf("CHARP: %s\n", SCM_CHARP(value) ? "true" : "false");
    printf("CHAR_VALUE: %c\n", (int)SCM_CHAR_VALUE(value));
    printf("\n");

    value = SCM_MAKE_STR("Hello, Gauche!");
    printf("STRINGP: %s\n", SCM_STRINGP(value) ? "true" : "false");
    printf("%s\n", Scm_GetString(SCM_STRING(value)));
    printf("\n");

    value = Scm_MakeFlonum(123.456);
    printf("FLONUMP: %s\n", SCM_FLONUMP(value) ? "true" : "false");
    printf("FLONUM_VALUE: %lf\n", SCM_FLONUM_VALUE(value));
    printf("\n");
}

void sample_list()
{
    ScmObj p;
    ScmObj list = Scm_Cons(SCM_MAKE_INT(1), Scm_Cons(SCM_MAKE_INT(2), Scm_Cons(SCM_MAKE_INT(3), SCM_NIL)));

    SCM_FOR_EACH(p, list) {
        printf("%ld\n", SCM_INT_VALUE(Scm_Car(p)));
    }
}

void sample_eval_cstring()
{
    const char *source = "(+ 10 20)";
    ScmEvalPacket eval_packet;

    if(Scm_EvalCString(source, SCM_OBJ(Scm_UserModule()), &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    printf("%ld\n", SCM_INT_VALUE(eval_packet.results[0]));
}

void sample_eval()
{
    ScmObj expression = SCM_LIST3(SCM_INTERN("+"), SCM_MAKE_INT(10), SCM_MAKE_INT(20));
    ScmEvalPacket eval_packet;

    if(Scm_Eval(expression, SCM_OBJ(Scm_UserModule()), &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    printf("%ld\n", SCM_INT_VALUE(eval_packet.results[0]));
}

void sample_apply()
{
    ScmEvalPacket eval_packet;

    /* 値を 2 乗する手続き square を定義. */
    if(Scm_EvalCString("(define square (lambda (x) (* x x)))", SCM_OBJ(Scm_UserModule()), &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }

    /* 定義した square を取得. */
    ScmObj procedure = Scm_GlobalVariableRef(Scm_UserModule(),
                                             SCM_SYMBOL(SCM_INTERN("square")),
                                             SCM_BINDING_STAY_IN_MODULE);

    /* (apply square '(3)) */
    if(Scm_Apply(procedure, SCM_LIST1(SCM_MAKE_INT(3)), &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }

    /* 結果を表示. */
    printf("%ld\n", SCM_INT_VALUE(eval_packet.results[0]));
}

void sample_load_from_port()
{
    ScmObj source = SCM_MAKE_STR(
        "(define sum (lambda (xs)"
        "    (apply + xs)))"
        "(define average (lambda (xs)"
        "    (/ (sum xs) (length xs))))"
    );
    ScmObj port = Scm_MakeInputStringPort(SCM_STRING(source), TRUE);

    ScmLoadPacket load_packet;
    ScmEvalPacket eval_packet;
    ScmObj procedure;
    ScmObj arguments;

    if(Scm_LoadFromPort(SCM_PORT(port), 0, &load_packet) < 0) {
        PRINT_EXCEPTION(load_packet);
        return;
    }

    /* (apply sum '((1 2 3))) */
    procedure = Scm_GlobalVariableRef(Scm_UserModule(), SCM_SYMBOL(SCM_INTERN("sum")), SCM_BINDING_STAY_IN_MODULE);
    arguments = SCM_LIST1(SCM_LIST3(SCM_MAKE_INT(1), SCM_MAKE_INT(2), SCM_MAKE_INT(3)));
    if(Scm_Apply(procedure, arguments, &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    print_object(eval_packet.results[0]);

    /* (apply average '((1 2 3))) */
    procedure = Scm_GlobalVariableRef(Scm_UserModule(), SCM_SYMBOL(SCM_INTERN("average")), SCM_BINDING_STAY_IN_MODULE);
    arguments = SCM_LIST1(SCM_LIST3(SCM_MAKE_INT(1), SCM_MAKE_INT(2), SCM_MAKE_INT(3)));
    if(Scm_Apply(procedure, arguments, &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    print_object(eval_packet.results[0]);
}

void sample_load()
{
    ScmLoadPacket load_packet;
    ScmEvalPacket eval_packet;
    ScmObj procedure;
    ScmObj arguments;

    Scm_AddLoadPath(".", TRUE);

    if(Scm_Load("sample.scm", 0, &load_packet) < 0) {
        PRINT_EXCEPTION(load_packet);
        return;
    }

    /* (apply sum '((1 2 3))) */
    procedure = Scm_GlobalVariableRef(Scm_UserModule(), SCM_SYMBOL(SCM_INTERN("sum")), SCM_BINDING_STAY_IN_MODULE);
    arguments = SCM_LIST1(SCM_LIST3(SCM_MAKE_INT(1), SCM_MAKE_INT(2), SCM_MAKE_INT(3)));
    if(Scm_Apply(procedure, arguments, &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    print_object(eval_packet.results[0]);

    /* (apply average '((1 2 3))) */
    procedure = Scm_GlobalVariableRef(Scm_UserModule(), SCM_SYMBOL(SCM_INTERN("average")), SCM_BINDING_STAY_IN_MODULE);
    arguments = SCM_LIST1(SCM_LIST3(SCM_MAKE_INT(1), SCM_MAKE_INT(2), SCM_MAKE_INT(3)));
    if(Scm_Apply(procedure, arguments, &eval_packet) < 0) {
        PRINT_EXCEPTION(eval_packet);
        return;
    }
    print_object(eval_packet.results[0]);
}

int main()
{
    GC_INIT();
    Scm_Init(GAUCHE_SIGNATURE);

    #define CALL_SAMPLE(name) {                            \
        printf("-------- %s --------\n", "sample_" #name); \
        sample_##name();                                   \
        printf("\n");                                      \
    }

    CALL_SAMPLE(basic);
    CALL_SAMPLE(list);
    CALL_SAMPLE(eval_cstring);
    CALL_SAMPLE(eval);
    CALL_SAMPLE(apply);
    CALL_SAMPLE(load_from_port);
    CALL_SAMPLE(load);

    #undef CALL_SAMPLE

    return 0;
}

