#include "shared.h"

/*************** INSTRUCCIONES ***************/
typedef struct { char *key; int val;} t_symstruct;

static t_symstruct lookUpTable[] = {
		{ "SET", I_SET },
		{ "MOV_IN", I_MOV_IN },
		{ "MOV_OUT", I_MOV_OUT },
		{ "I/O", I_IO },
		{ "F_OPEN", I_F_OPEN },
		{ "F_CLOSE", I_F_CLOSE },
		{ "F_SEEK", I_F_SEEK },
		{ "F_READ", I_F_READ },
		{ "F_WRITE", I_F_WRITE },
		{ "F_TRUNCATE", I_TRUNCATE },
		{ "WAIT", I_WAIT },
		{ "SIGNAL", I_SIGNAL },
		{ "CREATE_SEGMENT", I_CREATE_SEGMENT },
		{ "DELETE_SEGMENT", I_DELETE_SEGMENT },
		{ "YIELD", I_YIELD },
		{ "EXIT", I_EXIT },
};

int keyFromString(char *key) {
    int i;
    for (i=0; i < 16; i++) {
        t_symstruct sym = lookUpTable[i];
        if (strcmp(sym.key, key) == 0)
            return sym.val;
    }
    return BADKEY;
}
