#include "../include/conn_req_parser.h"
#include "../include/defines.h"
#include <stdio.h>
#include <stdlib.h>

static void version(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_VERSION, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_VERSION;
  ret->n = 1;
  ret->data[0] = c;
}

static void cmd(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_CMD, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_CMD;
  ret->n = 1;
  ret->data[0] = c;
}

static void rsv(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_RSV, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_RSV;
  ret->n = 1;
  ret->data[0] = c;
}

static void atyp(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_ATYP, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_ATYP;
  ret->n = 1;
  ret->data[0] = c;
}

static void dom_len(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_DOM_LEN, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_DOM_LEN;
  ret->n = 1;
  ret->data[0] = c;
}

static void dst_addr(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_DST_ADDR, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_DST_ADDR;
  ret->n = 1;
  ret->data[0] = c;
}

static void dst_port_byte1(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_DST_PORT_BYTE1, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_DST_PORT_BYTE1;
  ret->n = 1;
  ret->data[0] = c;
}

static void dst_port_byte2(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_DST_PORT_BYTE2, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_DST_PORT_BYTE2;
  ret->n = 1;
  ret->data[0] = c;
}

static void done(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_DONE, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_DONE;
  ret->n = 1;
  ret->data[0] = c;
}

static void error(struct parser_event *ret, const uint8_t c) {
  printf("STATE: CONN_REQ_ERROR, reading byte: 0x%x\n", c);
  ret->type = CONN_REQ_ERROR;
  ret->n = 1;
  ret->data[0] = c;
}

static const struct parser_state_transition version_transitions[] = {
    {
        .when = SOCKS5_VERSION,
        .dest = CONN_REQ_CMD,
        .act1 = version,
    },
    {
        .when = ANY,
        .dest = CONN_REQ_ERROR,
        .act1 = error,
    },
};

static const struct parser_state_transition cmd_transitions[] = {
    {
        .when = SOCKS5_CONN_REQ_CMD_CONNECT,
        .dest = CONN_REQ_RSV,
        .act1 = cmd,
    },
    {
        .when = SOCKS5_CONN_REQ_CMD_BIND,
        .dest = CONN_REQ_RSV,
        .act1 = cmd,
    },
    {
        .when = SOCKS5_CONN_REQ_CMD_UDP_ASSOCIATE,
        .dest = CONN_REQ_RSV,
        .act1 = cmd,
    },
    {
        .when = ANY,
        .dest = CONN_REQ_ERROR,
        .act1 = error,
    },
};

static const struct parser_state_transition rsv_transitions[] = {
    {
        .when = SOCKS5_CONN_REQ_RSV,
        .dest = CONN_REQ_ATYP,
        .act1 = rsv,
    },
    {
        .when = ANY,
        .dest = CONN_REQ_ERROR,
        .act1 = error,
    },
};

static const struct parser_state_transition atyp_transitions[] = {
    {
        .when = SOCKS5_CONN_REQ_ATYP_IPV4,
        .dest = CONN_REQ_DST_ADDR,
        .act1 = atyp,
    },
    {
        .when = SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME,
        .dest = CONN_REQ_DOM_LEN,
        .act1 = atyp,
    },
    {
        .when = SOCKS5_CONN_REQ_ATYP_IPV6,
        .dest = CONN_REQ_DST_ADDR,
        .act1 = atyp,
    },
    {
        .when = ANY,
        .dest = CONN_REQ_ERROR,
        .act1 = error,
    },
};

static const struct parser_state_transition dom_len_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_DST_ADDR,
        .act1 = dom_len,
    },
};

static const struct parser_state_transition dst_addr_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_DST_ADDR,
        .act1 = dst_addr,
    },
};

static const struct parser_state_transition dst_port_byte1_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_DST_PORT_BYTE2,
        .act1 = dst_port_byte1,
    },
};

static const struct parser_state_transition dst_port_byte2_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_DONE,
        .act1 = dst_port_byte2,
    },
};

static const struct parser_state_transition done_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_DONE,
        .act1 = done,
    },
};

static const struct parser_state_transition error_transitions[] = {
    {
        .when = ANY,
        .dest = CONN_REQ_ERROR,
        .act1 = error,
    },
};

static const struct parser_state_transition *states[] = {
    [CONN_REQ_VERSION] = version_transitions,
    [CONN_REQ_CMD] = cmd_transitions,
    [CONN_REQ_RSV] = rsv_transitions,
    [CONN_REQ_ATYP] = atyp_transitions,
    [CONN_REQ_DOM_LEN] = dom_len_transitions,
    [CONN_REQ_DST_ADDR] = dst_addr_transitions,
    [CONN_REQ_DST_PORT_BYTE1] = dst_port_byte1_transitions,
    [CONN_REQ_DST_PORT_BYTE2] = dst_port_byte2_transitions,
    [CONN_REQ_DONE] = done_transitions,
    [CONN_REQ_ERROR] = error_transitions,
};

static const size_t states_n[] = {
    [CONN_REQ_VERSION] = N(version_transitions),
    [CONN_REQ_CMD] = N(cmd_transitions),
    [CONN_REQ_RSV] = N(rsv_transitions),
    [CONN_REQ_ATYP] = N(atyp_transitions),
    [CONN_REQ_DOM_LEN] = N(dom_len_transitions),
    [CONN_REQ_DST_ADDR] = N(dst_addr_transitions),
    [CONN_REQ_DST_PORT_BYTE1] = N(dst_port_byte1_transitions),
    [CONN_REQ_DST_PORT_BYTE2] = N(dst_port_byte2_transitions),
    [CONN_REQ_DONE] = N(done_transitions),
    [CONN_REQ_ERROR] = N(error_transitions),
};

static const struct parser_definition conn_req_parser_definition = {
    .states = states,
    .states_n = states_n,
    .start_state = CONN_REQ_VERSION,
};

conn_req_parser *conn_req_parser_init(void) {
  conn_req_parser *p = malloc(sizeof(conn_req_parser));
  if (p == NULL) {
    perror("malloc");
    return NULL;
  }
  p->parser = parser_init(parser_no_classes(), &conn_req_parser_definition);
  p->cmd = 0;            // Comando inicial
  p->atyp = 0;           // Tipo de dirección inicial
  p->dst_addr_len = 0;   // Longitud del dominio destino inicial
  p->dst_addr_count = 0; // Contador de bytes leídos de dst_addr inicial
  p->dst_port = 0;       // Puerto destino inicial
  return p;
}

conn_req_parser_state conn_req_parser_feed(conn_req_parser *p, uint8_t byte) {
  const struct parser_event *e = parser_feed(p->parser, byte);

  switch (e->type) {
  case CONN_REQ_CMD:
    p->cmd = e->data[0];
    break;
  case CONN_REQ_ATYP:
    p->atyp = e->data[0];
    switch (p->atyp) {
    case SOCKS5_CONN_REQ_ATYP_IPV4:
      p->dst_addr_len = 4;
      break;
    case SOCKS5_CONN_REQ_ATYP_DOMAIN_NAME:
      break;
    case SOCKS5_CONN_REQ_ATYP_IPV6:
      p->dst_addr_len = 16;
      break;
    default:
      return CONN_REQ_ERROR;
    }
    break;
  case CONN_REQ_DOM_LEN:
    p->dst_addr_len = e->data[0];
    break;
  case CONN_REQ_DST_ADDR:
    p->dst_addr[p->dst_addr_count++] = e->data[0];
    if (p->dst_addr_count >= p->dst_addr_len) {
      parser_set(p->parser, CONN_REQ_DST_PORT_BYTE1);
      return CONN_REQ_DST_PORT_BYTE1;
    }
    break;
  case CONN_REQ_DST_PORT_BYTE1:
  case CONN_REQ_DST_PORT_BYTE2:
    p->dst_port = (p->dst_port << 8) | e->data[0];
    break;
  }
  return e->type;
}

void conn_req_parser_close(conn_req_parser *p) { parser_destroy(p->parser); }

int main(void) {
  // TESTING CONNECTION REQUEST PARSER
  conn_req_parser *p = conn_req_parser_init();
  if (p == NULL) {
    perror("conn_req_parser_init");
    return 1;
  }
  uint8_t mock_buffer[] = {0x05, 0x01, 0x00, 0x03, 14,  'w', 'w', 'w', '.', 'g',
                           'o',  'o',  'g',  'l',  'e', '.', 'c', 'o', 'm'};

  for (size_t i = 0; i < sizeof(mock_buffer); i++) {
    conn_req_parser_feed(p, mock_buffer[i]);
  }

  printf("cmd: %d\n", p->cmd);
  printf("atyp: %d\n", p->atyp);
  printf("dst_addr_len: %d\n", p->dst_addr_len);
  printf("dst_addr: ");
  for (size_t i = 0; i < p->dst_addr_len; i++) {
    printf("%c", p->dst_addr[i]);
  }
  printf("\n");
  printf("dst_port: %d\n", p->dst_port);

  conn_req_parser_close(p);
}