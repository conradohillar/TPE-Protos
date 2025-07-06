#include <main.h>

static server_data_t * _server_data = NULL;

server_data_t *get_server_data() {
  return _server_data;
}

static bool done = false;

static void sigterm_handler(int signum) {
  log_info("Received signal %d", signum);
  done = true;
}

int create_and_register_passive_socket(fd_selector * selector, char * address, unsigned short port, const fd_handler * callback_functions, selector_status * ss, const char ** error_msg, char * protocol){

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));

  addr.sin_port = htons(port); 
  addr.sin_family = AF_INET;
  if(address != NULL) {
    if(inet_pton(AF_INET, address, &addr.sin_addr) <= 0) {
      *error_msg = "Invalid address format for socks_addr";
      goto error;
    }
  } else {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  const int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(fd < 0) {
    log_error("Error creating socket");
    *error_msg = "Error creating socket";
    goto error;
  }

  log_info("Server listening on TCP port %d - Protocol: %s", port, protocol);

  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

  if(bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    log_error("Error binding socket");
    *error_msg = "Error binding socket";
    goto error;
  }

  if(listen(fd, MAX_PENDING_CONNECTIONS) < 0) {
    log_error("Error listening on socket");
    *error_msg = "Error listening on socket";
    goto error;
  }


  *ss = set_non_blocking_fd(fd);
  if(*ss != SELECTOR_SUCCESS) {
    log_error("Error setting server socket to non-blocking");
    *error_msg = "Error setting server socket to non-blocking";
    goto error;
  }


  *ss = selector_register(*selector, fd, callback_functions, OP_READ, NULL);
  if(*ss != SELECTOR_SUCCESS) {
    log_error("Error registering server socket with selector");
    *error_msg = "Error registering server socket with selector";
    goto error;
  }

  return 0;

  error:
    if(fd < 0) close(fd);
    return -1;
}

int main(int argc, char *argv[]) {
  log_info("Starting SOCKS5 server");
  close(FD_STDIN);

  server_args args = { 0 };
  parse_args(argc, argv, &args);

  signal(SIGINT, sigterm_handler);
  signal(SIGTERM, sigterm_handler);
  
  log_info("Initializing authentication system");
  auth_init();
  for(int i = 0; i < args.users_count; i++) {
    log_debug("Adding user: %s", args.users[i].name);
    auth_add_user(args.users[i].name, args.users[i].pass);
  }

  _server_data = malloc(sizeof(server_data_t));
  if (_server_data == NULL) {
    log_error("Failed to allocate memory for server data");
    perror("Failed to allocate memory for server data");
    exit(EXIT_FAILURE);
  }
  log_debug("Server data allocated successfully");
  _server_data->metrics = metrics_init();
  _server_data->access_register = access_register_init();

  const char * error_msg = NULL;
  fd_selector fd_selector = NULL;
  selector_status ss = SELECTOR_SUCCESS;

  const struct selector_init config = {
    .signal = SIGALRM,
    .select_timeout = { .tv_sec = SEC_TIMEOUT, .tv_nsec = NANO_SEC_TIMEOUT },
  };

  ss = selector_init(&config);
  if(ss != SELECTOR_SUCCESS) {
    log_error("Error initializing selector");
    error_msg = "Error initializing selector";
    goto error;
  }

  log_debug("Selector initialized successfully");

  fd_selector = selector_new(INITIAL_QUANTITY_FDS);
  if(fd_selector == NULL) {
    log_error("Error creating selector");
    error_msg = "Error creating selector";
    goto error;
  }

  log_debug("Selector created successfully");

  const struct fd_handler socksv5 = {
    .handle_read       = socksv5_passive_accept,
  };

  const struct fd_handler conf_protocol = {
    .handle_read       = s5admin_passive_accept
  };

  if(create_and_register_passive_socket(&fd_selector, args.socks_addr, args.socks_port, &socksv5, &ss, &error_msg, SOCKS5) == -1) goto error;
  if(create_and_register_passive_socket(&fd_selector, args.mng_addr, args.mng_port, &conf_protocol, &ss, &error_msg, CONF_PROTOCOL) == -1) goto error;

  log_info("Server started successfully, entering main loop");
  while(!done) {
    ss = selector_select(fd_selector);
    if(ss != SELECTOR_SUCCESS) {
      log_error("Error during selector select");
      error_msg = "Error during selector select";
      goto error;
    }
  }
  
  if(error_msg == NULL) error_msg = "closing";
  int ret = 0;

  error:
  if(ss != SELECTOR_SUCCESS) {
    log_error("%s: %s", (error_msg == NULL) ? "": error_msg, ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
    fprintf(stderr, "%s: %s\n", (error_msg == NULL) ? "": error_msg, ss == SELECTOR_IO ? strerror(errno) : selector_error(ss));
    ret = 2;
  } else if(error_msg) {
    log_error("%s", error_msg);
    perror(error_msg);
    ret = 1;
  }
  
  log_info("Shutting down server");
  if(fd_selector != NULL) selector_destroy(fd_selector);
  
  selector_close();

  free(_server_data->access_register);
  free(_server_data->metrics);
  free(_server_data);

  auth_destroy();

  log_info("Server shutdown complete");
  return ret;
}
