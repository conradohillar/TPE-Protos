#include <server.h>

static bool done = false;

static void
sigterm_handler(int signum) {
    LOG(DEBUG, "Received signal %d", signum);
    done = true;
}

int main(int argc, char* argv[]) {
    server_args args = {0};
    parse_args(argc, argv, &args);

    init_logging(NULL, args.log_level);
    server_data_init();
    auth_init();

    close(FD_STDIN);

    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);

    for (int i = 0; i < args.users_count; i++) {
        LOG(DEBUG, "Adding user: %s", args.users[i].name);
        auth_add_user(args.users[i].name, args.users[i].pass);
    }

    const char* error_msg = NULL;
    fd_selector fd_selector = NULL;
    selector_status ss = SELECTOR_SUCCESS;

    const struct selector_init config = {
        .signal = SIGALRM,
        .select_timeout = {.tv_sec = SEC_TIMEOUT, .tv_nsec = NANO_SEC_TIMEOUT},
    };

    ss = selector_init(&config);
    if (ss != SELECTOR_SUCCESS) {
        LOG_MSG(ERROR, "Error initializing selector");
        error_msg = "Error initializing selector";
        goto error;
    }

    LOG_MSG(DEBUG, "Selector initialized successfully");

    fd_selector = selector_new(INITIAL_QUANTITY_FDS);
    if (fd_selector == NULL) {
        LOG_MSG(ERROR, "Error creating selector");
        error_msg = "Error creating selector";
        goto error;
    }

    LOG_MSG(DEBUG, "Selector created successfully");

    const struct fd_handler socksv5 = {
        .handle_read = socksv5_passive_accept,
    };

    const struct fd_handler conf_protocol = {.handle_read = s5admin_passive_accept};

    if (create_and_register_passive_socket(&fd_selector, args.socks_addr, args.socks_port, &socksv5, &ss, &error_msg, SOCKS5) == -1)
        goto error;
    if (create_and_register_passive_socket(&fd_selector, args.mng_addr, args.mng_port, &conf_protocol, &ss, &error_msg, CONF_PROTOCOL) == -1)
        goto error;

    LOG_MSG(INFO, "Server started successfully, entering main loop");
    while (!done) {
        ss = selector_select(fd_selector);
        if (ss != SELECTOR_SUCCESS) {
            LOG(ERROR, "Error during selector select: %s", selector_error(ss));
            error_msg = "Error during selector select";
            goto error;
        }
    }

    int ret = 0;

error:
    if (ss != SELECTOR_SUCCESS) {
        ret = EXIT_FAILURE;
    }

    LOG_MSG(INFO, "Shutting down server");
    if (fd_selector != NULL)
        selector_destroy(fd_selector);

    selector_close();

    auth_destroy();

    server_data_destroy();

    LOG_MSG(INFO, "Server shutdown complete");
    close_logging();
    return ret;
}
