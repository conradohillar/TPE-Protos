#include <conn_req.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>
#include <conn_req_parser.h>
#include <netutils.h>

// Inicializa el buffer y offsets para la etapa de request
void connection_req_on_arrival(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    log_debug("Starting connection request phase for fd %d", key->fd);
    conn->conn_req_parser = conn_req_parser_init();
}

// Lee y parsea el mensaje de request SOCKS5
unsigned int connection_req_read(struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    while (buffer_can_read(&conn->in_buff)) {
        
        conn_req_parser_state state = conn_req_parser_feed(conn->conn_req_parser, buffer_read(&conn->in_buff));

        log_info("Processing connection request for fd %d, state: %d", key->fd, state);
        if (state == CONN_REQ_DONE) {
            log_info("Connection request received for fd %d, connecting to destination", key->fd);
            log_info("Destination address: %s, port: %d", conn->conn_req_parser->dst_addr, conn->conn_req_parser->dst_port);
            
            //esto lo tenemos que hacer en otro thread pero primero quiero ver que funque asi
            conn->origin_fd = connect_to_host(conn->conn_req_parser->dst_addr, conn->conn_req_parser->dst_port);
    
            if (conn->origin_fd < 0) {
                // Error al conectar al destino
                log_error("Error connecting to destination for fd %d", key->fd);
                perror("error connecting to destination");
                return SOCKS5_ERROR;
            }//aca podriamos manejar mas errores esto es algo basico

            log_info("Successfully connected to destination for fd %d, origin_fd: %d", key->fd, conn->origin_fd);
            buffer_write(&conn->out_buff, SOCKS5_VERSION); 
            buffer_write(&conn->out_buff, SOCKS5_SUCCESS); 
            buffer_write(&conn->out_buff, SOCKS5_CONN_REQ_RSV); // RSV
            buffer_write(&conn->out_buff, ATYP_IPV4); //ESTO LO DEBERIAMOS TENER GUARDADO

          
            //memset(conn->out_buff.data+4, 0, 6); // BND.ADDR + BND.PORT (0.0.0.0:0)
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_COPY;

        } else if (state == CONN_REQ_ERROR) {
            log_error("Connection request error for fd %d", key->fd);
            //aca falta hacer todo el manejo de errores
             buffer_write(&conn->out_buff, SOCKS5_VERSION); 
            buffer_write(&conn->out_buff, SOCKS5_GENERAL_FAILURE); 
            buffer_write(&conn->out_buff, 0x00); 
            buffer_write(&conn->out_buff, ATYP_IPV4); 
            //memset(conn->out_buff.data+4, 0, 6);
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&conn->in_buff);
            return SOCKS5_ERROR;
        }
    }
    return SOCKS5_CONNECTION_REQ;
}

void connection_req_on_departure(unsigned state, struct selector_key *key) {
    socks5_conn_t *conn = key->data;
    log_debug("Connection request phase complete for fd %d", key->fd);
    conn_req_parser_close(conn->conn_req_parser);
    conn->conn_req_parser = NULL;
}