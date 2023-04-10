t_log* iniciar_logger(char*);
t_config* iniciar_config(char*);
void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);
int armar_conexion(t_config*, t_log*);
