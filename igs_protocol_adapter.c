/*
 * IGS Protocol Adapter Implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mymalloc.h>
#include "igs_protocol_adapter.h"

#define INITIAL_BUFFER_SIZE 8192

/* Debug output - can be disabled by defining ADAPTER_QUIET */
#ifndef ADAPTER_QUIET
#define ADAPTER_DEBUG(fmt, ...) printf("ADAPTER: " fmt "\n", ##__VA_ARGS__)
#else
#define ADAPTER_DEBUG(fmt, ...)
#endif

IGSProtocolAdapter* adapter_init(void) {
    IGSProtocolAdapter *adapter = mynew(IGSProtocolAdapter);
    adapter->state = ADAPTER_WAITING_FOR_BANNER_END;
    adapter->buffer = mynews(char, INITIAL_BUFFER_SIZE);
    adapter->buffer_len = 0;
    adapter->buffer_capacity = INITIAL_BUFFER_SIZE;
    adapter->banner_complete = 0;
    
    ADAPTER_DEBUG("Initialized - waiting for IGS banner end");
    return adapter;
}

void adapter_free(IGSProtocolAdapter *adapter) {
    if (adapter) {
        if (adapter->buffer) {
            myfree(adapter->buffer);
        }
        myfree(adapter);
    }
}

void adapter_reset(IGSProtocolAdapter *adapter) {
    if (adapter) {
        adapter->state = ADAPTER_WAITING_FOR_BANNER_END;
        adapter->buffer_len = 0;
        adapter->banner_complete = 0;
        ADAPTER_DEBUG("Reset to initial state - waiting for IGS banner end");
    }
}

static void ensure_buffer_capacity(IGSProtocolAdapter *adapter, int needed_size) {
    if (adapter->buffer_capacity < needed_size) {
        int new_capacity = needed_size * 2;
        char *new_buffer = mynews(char, new_capacity);
        memcpy(new_buffer, adapter->buffer, adapter->buffer_len);
        myfree(adapter->buffer);
        adapter->buffer = new_buffer;
        adapter->buffer_capacity = new_capacity;
    }
}

/* Look for the end of the IGS ASCII banner - check for common end patterns */
static int detect_banner_end(const char *data, int len) {
    /* Strategy: Look for the end of ASCII content followed by binary data or specific end markers */
    
    /* First, try to find the binary pattern we saw in q5Go: "ÿü\u0001#> " */
    if (len >= 6) {
        for (int i = 0; i <= len - 6; i++) {
            if ((unsigned char)data[i] == 0xFF && 
                (unsigned char)data[i+1] == 0xFC &&
                (unsigned char)data[i+2] == 0x01 && 
                (unsigned char)data[i+3] == 0x23 &&
                data[i+4] == '>' && 
                data[i+5] == ' ') {
                return i + 6; /* Return position after the prompt */
            }
        }
    }
    
    /* Alternative: Look for common IGS banner end patterns */
    /* Simple approach: if we see "this software" and buffer is reasonably large, consider banner complete */
    const char *software_marker = "this software";
    int marker_len = strlen(software_marker);
    
    if (len > 1000) { /* Only check if buffer is substantial */
        for (int i = 0; i <= len - marker_len; i++) {
            if (memcmp(data + i, software_marker, marker_len) == 0) {
                /* Found software marker, assume banner ends shortly after */
                int end_pos = i + marker_len + 20; /* Allow some additional text */
                if (end_pos > len) end_pos = len;
                return end_pos;
            }
        }
    }
    
    return -1; /* Pattern not found */
}

char* adapter_process_server_data(IGSProtocolAdapter *adapter, const char *raw_data, 
                                  int raw_len, int *output_len) {
    *output_len = 0;
    
    if (!raw_data || raw_len <= 0) {
        return NULL;
    }
    
    switch (adapter->state) {
        case ADAPTER_WAITING_FOR_BANNER_END: {
            /* Buffer the incoming data and look for banner end */
            ADAPTER_DEBUG("Buffering %d bytes, total buffer now %d bytes", 
                         raw_len, adapter->buffer_len + raw_len);
            ensure_buffer_capacity(adapter, adapter->buffer_len + raw_len);
            memcpy(adapter->buffer + adapter->buffer_len, raw_data, raw_len);
            adapter->buffer_len += raw_len;
            
            /* Debug: Look for the binary prompt in the last 100 bytes if buffer is large */
            if (adapter->buffer_len > 1000) {
                ADAPTER_DEBUG("Large buffer (%d bytes), looking for banner end pattern...", adapter->buffer_len);
                int start = adapter->buffer_len - 100;
                if (start < 0) start = 0;
                
                ADAPTER_DEBUG("Last 50 bytes as hex:");
                for (int i = adapter->buffer_len - 50; i < adapter->buffer_len && i >= 0; i++) {
                    printf("%02X ", (unsigned char)adapter->buffer[i]);
                    if ((i - (adapter->buffer_len - 50) + 1) % 16 == 0) printf("\n");
                }
                printf("\n");
                fflush(stdout);
            }
            
            int prompt_pos = detect_banner_end(adapter->buffer, adapter->buffer_len);
            if (prompt_pos >= 0) {
                ADAPTER_DEBUG("Banner end detected at position %d", prompt_pos);
                
                /* Send ONLY the injected login prompt to the parser, not the banner */
                const char *login_prompt = "Login: ";
                int login_prompt_len = strlen(login_prompt);
                
                *output_len = login_prompt_len;
                char *output = mynews(char, *output_len + 1);
                
                /* Send only the synthetic login prompt */
                memcpy(output, login_prompt, login_prompt_len);
                output[*output_len] = '\0';
                
                ADAPTER_DEBUG("Sending only 'Login: ' prompt to parser, ignoring %d bytes of banner", prompt_pos);
                
                /* Store any remaining data for next call */
                int remaining = adapter->buffer_len - prompt_pos;
                if (remaining > 0) {
                    memmove(adapter->buffer, adapter->buffer + prompt_pos, remaining);
                }
                adapter->buffer_len = remaining;
                
                adapter->state = ADAPTER_INJECTED_LOGIN_PROMPT;
                ADAPTER_DEBUG("Injected 'Login: ' prompt, state -> INJECTED_LOGIN_PROMPT");
                
                return output;
            }
            
            /* Banner not complete yet, return nothing */
            return NULL;
        }
        
        case ADAPTER_INJECTED_LOGIN_PROMPT:
            /* We're waiting for XGospel to send "guest", just buffer server data */
            ensure_buffer_capacity(adapter, adapter->buffer_len + raw_len);
            memcpy(adapter->buffer + adapter->buffer_len, raw_data, raw_len);
            adapter->buffer_len += raw_len;
            return NULL;
            
        case ADAPTER_SENT_GUEST: {
            /* After sending "guest", inject confirmation and switch to passthrough */
            const char *guest_confirmation = "This is a guest account. Please see 'help register' to register.\n";
            int confirm_len = strlen(guest_confirmation);
            
            *output_len = confirm_len + raw_len + adapter->buffer_len;
            char *output = mynews(char, *output_len + 1);
            
            /* First, add the guest confirmation */
            memcpy(output, guest_confirmation, confirm_len);
            
            /* Then add any buffered data */
            int buffered_len = adapter->buffer_len;
            if (buffered_len > 0) {
                memcpy(output + confirm_len, adapter->buffer, buffered_len);
                adapter->buffer_len = 0;
            }
            
            /* Finally, add current raw data */
            if (raw_len > 0) {
                memcpy(output + confirm_len + buffered_len, raw_data, raw_len);
            }
            
            output[*output_len] = '\0';
            
            adapter->state = ADAPTER_PASSTHROUGH;
            ADAPTER_DEBUG("Injected guest confirmation, state -> PASSTHROUGH");
            
            return output;
        }
        
        case ADAPTER_REGISTERED_USER: {
            /* Registered user - pass through server data but filter out duplicate "Login: " prompts */
            int total_len = raw_len + adapter->buffer_len;
            char *temp_buffer = mynews(char, total_len + 1);
            
            /* Combine buffered and new data */
            int buffered_len = adapter->buffer_len;
            if (buffered_len > 0) {
                memcpy(temp_buffer, adapter->buffer, buffered_len);
                adapter->buffer_len = 0;
            }
            if (raw_len > 0) {
                memcpy(temp_buffer + buffered_len, raw_data, raw_len);
            }
            temp_buffer[total_len] = '\0';
            
            /* Filter out any "Login: " prompts to prevent duplicate username sending */
            char *filtered = mynews(char, total_len + 1);
            int filtered_len = 0;
            int i = 0;
            
            while (i < total_len) {
                /* Look for "Login: " pattern */
                if (i <= total_len - 7 && strncmp(temp_buffer + i, "Login: ", 7) == 0) {
                    ADAPTER_DEBUG("Filtering out duplicate 'Login: ' prompt for registered user");
                    i += 7; /* Skip the "Login: " */
                } else {
                    filtered[filtered_len++] = temp_buffer[i++];
                }
            }
            
            myfree(temp_buffer);
            
            if (filtered_len > 0) {
                filtered[filtered_len] = '\0';
                *output_len = filtered_len;
                
                adapter->state = ADAPTER_PASSTHROUGH;
                ADAPTER_DEBUG("Filtered server data for registered user, state -> PASSTHROUGH");
                return filtered;
            } else {
                myfree(filtered);
                return NULL;
            }
        }
        
        case ADAPTER_PASSTHROUGH:
            /* Normal operation - pass data through unchanged */
            *output_len = raw_len;
            char *output = mynews(char, raw_len + 1);
            memcpy(output, raw_data, raw_len);
            output[raw_len] = '\0';
            return output;
            
        default:
            ADAPTER_DEBUG("Unknown state: %d", adapter->state);
            return NULL;
    }
}

char* adapter_process_client_data(IGSProtocolAdapter *adapter, const char *client_data, 
                                  int client_len, int *output_len) {
    *output_len = 0;
    
    if (!client_data || client_len <= 0) {
        return NULL;
    }
    
    /* Check what username the client is sending */
    if (adapter->state == ADAPTER_INJECTED_LOGIN_PROMPT) {
        if (strncmp(client_data, "guest", 5) == 0) {
            ADAPTER_DEBUG("Client sent 'guest', state -> SENT_GUEST");
            adapter->state = ADAPTER_SENT_GUEST;
        } else {
            /* Non-guest username - registered user */
            ADAPTER_DEBUG("Client sent registered username, state -> REGISTERED_USER");
            adapter->state = ADAPTER_REGISTERED_USER;
        }
    }
    
    /* Pass client data through unchanged */
    *output_len = client_len;
    char *output = mynews(char, client_len + 1);
    memcpy(output, client_data, client_len);
    output[client_len] = '\0';
    
    return output;
}