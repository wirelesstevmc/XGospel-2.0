/*
 * IGS Protocol Adapter for XGospel
 * 
 * This adapter acts as a translation layer between the modern IGS server
 * and XGospel's 30-year-old yacc parser, injecting synthetic login prompts
 * that the parser expects.
 */

#ifndef IGS_PROTOCOL_ADAPTER_H
#define IGS_PROTOCOL_ADAPTER_H

typedef enum {
    ADAPTER_WAITING_FOR_BANNER_END,    /* Waiting for server banner to complete */
    ADAPTER_INJECTED_LOGIN_PROMPT,     /* Injected "Login: " and waiting for response */
    ADAPTER_SENT_GUEST,                /* Sent "guest" to server, inject confirmation */
    ADAPTER_REGISTERED_USER,           /* Registered user, pass server data through */
    ADAPTER_PASSTHROUGH                /* Normal operation, pass all data through */
} AdapterState;

typedef struct {
    AdapterState state;
    char *buffer;           /* Buffer for incomplete data */
    int buffer_len;         /* Current buffer length */
    int buffer_capacity;    /* Buffer capacity */
    int banner_complete;    /* Flag indicating banner end detected */
} IGSProtocolAdapter;

/* Initialize the protocol adapter */
IGSProtocolAdapter* adapter_init(void);

/* Free the protocol adapter */
void adapter_free(IGSProtocolAdapter *adapter);

/* Process incoming data from IGS server, return adapted data for yacc parser */
char* adapter_process_server_data(IGSProtocolAdapter *adapter, const char *raw_data, 
                                  int raw_len, int *output_len);

/* Process outgoing data from XGospel to IGS server */
char* adapter_process_client_data(IGSProtocolAdapter *adapter, const char *client_data, 
                                  int client_len, int *output_len);

#endif /* IGS_PROTOCOL_ADAPTER_H */