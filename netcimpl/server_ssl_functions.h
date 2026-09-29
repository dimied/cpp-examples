#ifndef SERVER_SSL_FUNCTIONS_INCLUDED
#define SERVER_SSL_FUNCTIONS_INCLUDED

// TODO: implement SSL_CTX_config
#if 0
if (CONF_modules_load_file("config.cnf", "testapp", 0) <= 0) {
    fprintf(stderr, "Error processing config file\n");
    goto err;
}

ctx = SSL_CTX_new(TLS_server_method());

if (SSL_CTX_config(ctx, "server") == 0) {
    fprintf(stderr, "Error configuring server.\n");
    goto err;
}
#endif

static int handleSSLError(MyServerConnection *pConnection, int err);

static int prepareSSL(MyServerConnection *pConnection)
{
    if (pConnection != 0)
    {
        if ((pConnection->options & SERVER_OPTION_SECURE) != 0)
        {
            // test if files are accessible
            int ssl_error = 0;
            int fr = access(pConnection->secure.certFilePath, F_OK | R_OK);
            if (fr != 0)
            {
                return handleSSLError(pConnection, 100);
            }
            fr = access(pConnection->secure.privKeyFilePath, F_OK | R_OK);
            if (fr != 0)
            {
                return handleSSLError(pConnection, 200);
            }

            /*@shared@*/ const SSL_METHOD *m = TLS_server_method();
            /*@dependent@*/ SSL_CTX *ctx = SSL_CTX_new(m);

            if (0 == ctx)
            {
                return handleSSLError(pConnection, 1);
            }
            else
            {
                int r = SSL_CTX_use_certificate_chain_file(ctx, pConnection->secure.certFilePath);
                if (1 == r)
                {
                    r = SSL_CTX_use_PrivateKey_file(ctx, pConnection->secure.privKeyFilePath, SSL_FILETYPE_PEM);
                    if (1 != r)
                    {
                        ssl_error = handleSSLError(pConnection, 3);
                        SSL_CTX_free(ctx);
                    }
                }
                else
                {
                    ssl_error = handleSSLError(pConnection, 2);
                    SSL_CTX_free(ctx);
                }
            }

            if (0 == ssl_error)
            {

                if (((SSL_CTX *)0) != pConnection->secure.ctx)
                {
                    SSL_CTX *old_ctx = pConnection->secure.ctx;
                    // SSL_CTX_free(/*@special@*/ /*@notnull@*/old_ctx) /*@releases old_ctx@*/;
                    SSL_CTX_free(old_ctx);
                    pConnection->secure.ctx = (SSL_CTX *)0;
                }
                pConnection->secure.ctx = ctx;
            }
            else
            {
                pConnection->common.status = ssl_error;
                return ssl_error;
            }
        }
        return 0;
    }
    return 1;
}

#ifndef S_SPLINT_S
static int errorCB(const char *str, size_t len, /*@temp@*/ void *_)
{
    (void)(_);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    (void)write(STDOUT_FILENO, str, len);
    (void)write(STDOUT_FILENO, "\n", 1);
#pragma GCC diagnostic pop
    // positive value to continue, 0 to stop
    return 1;
}
#endif

static int handleSSLError(MyServerConnection *pConnection, int err)
{
    //(void)(pConnection);
    unsigned long lastError = ERR_get_error();
    printf("LAST-ERROR: %lu|%d|%p\n", lastError, err, (void *)pConnection);

#ifndef S_SPLINT_S
    ERR_print_errors_cb(errorCB, /*@null@*/ (void *)0);
#endif
    return err;
}

#endif