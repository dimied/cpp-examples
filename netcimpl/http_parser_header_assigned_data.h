#ifndef HTTP_HEADER_ASSIGNED_DATA
#define HTTP_HEADER_ASSIGNED_DATA

#define DESCRIBE_HEADER_HANDLER(S, H) {S, H}

HeaderHandler headerHandlers[] = {
    DESCRIBE_HEADER_HANDLER("accept", parseAcceptCC),
    DESCRIBE_HEADER_HANDLER("accept-encoding", parseAcceptEncodingCC),
    DESCRIBE_HEADER_HANDLER("accept-language", parseAcceptLanguageCC),
    DESCRIBE_HEADER_HANDLER("access-control-request-headers", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("access-control-request-method", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("alt-used", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("authorization", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("available-dictionary", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("cache-control", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("connection", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-disposition", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("content-encoding", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-length", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-location", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("content-type", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("cookie", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("date", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dictionary-id", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("early-data", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("expect", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("forwarded", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("from", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("idempotency-key", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("if-match", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-modified-since", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-none-match", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("if-range", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("if-unmodified-since", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("keep-alive", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("link", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("max-forwards", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("origin", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("pragma", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("prefer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("priority", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("proxy-authorization", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("range", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("referer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("repr-digest", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("rtt", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("save-data", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("service-worker", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("service-worker-navigation-preload", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("te", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("trailer", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("transfer-encoding", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("upgrade", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("upgrade-insecure-requests", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("user-agent", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("via", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("want-content-digest", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("want-repr-digest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("x-forwarded-for", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("x-forwarded-host", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("x-forwarded-proto", parseNotImplemented),
};


HeaderHandler headerHandlersSec[] = {
    DESCRIBE_HEADER_HANDLER("sec-fetch-dest", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-mode", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-site", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-storage-access", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-fetch-user", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-gpc", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-private-state-token", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-private-state-token-crypto-version", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-purpose", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-purpose-redemption-record", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-speculation-tags", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-extensions", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-key", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-protocol", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-websocket-version", parseNotImplemented),
};
#define NUM_HEADER_HANDLERS_SEC (sizeof(headerHandlersSec) / sizeof(headerHandlersSec[0]))

HeaderHandler headerHandlersClientHints[] = {
    DESCRIBE_HEADER_HANDLER("device-memory", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dnt", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("downlink", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("dpr", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("ect", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-device-memory", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-dpr", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-prefers-color-scheme", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("sec-ch-prefers-reduced-motion", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-prefers-reduced-transparency", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-arch", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-bitness", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-form-factors", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-full-version", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-full-version-list", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-mobile", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-model", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-platform", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-ua-platform-version", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("sec-ch-viewport-height", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-viewport-width", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("sec-ch-width", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("viewport-width", parseNotImplemented),
    /**/
    DESCRIBE_HEADER_HANDLER("warning", parseNotImplemented),
    DESCRIBE_HEADER_HANDLER("width", parseNotImplemented),
};

#undef DESCRIBE_HEADER_HANDLER

#define NUM_HEADER_HANDLERS_CLIENT_HINTS (sizeof(headerHandlersClientHints) / sizeof(headerHandlersClientHints[0]))

#if 0
const char headerNamesAligned8[872] = {
    'a', 'c', 'c', 'e', 'p', 't', 0, 0,                                                                                                                                                       /**/
    'a', 'c', 'c', 'e', 'p', 't', '-', 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g', 0,                                                                                                             /**/
    'a', 'c', 'c', 'e', 'p', 't', '-', 'l', 'a', 'n', 'g', 'u', 'a', 'g', 'e', 0,                                                                                                             /**/
    'a', 'c', 'c', 'e', 's', 's', '-', 'c', 'o', 'n', 't', 'r', 'o', 'l', '-', 'r', 'e', 'q', 'u', 'e', 's', 't', '-', 'h', 'e', 'a', 'd', 'e', 'r', 's', 0, 0,                               /**/
    'a', 'c', 'c', 'e', 's', 's', '-', 'c', 'o', 'n', 't', 'r', 'o', 'l', '-', 'r', 'e', 'q', 'u', 'e', 's', 't', '-', 'm', 'e', 't', 'h', 'o', 'd', 0, 0, 0,                                 /**/
    'a', 'l', 't', '-', 'u', 's', 'e', 'd',                                                                                                                                                   /**/
    'a', 'u', 't', 'h', 'o', 'r', 'i', 'z', 'a', 't', 'i', 'o', 'n', 0, 0, 0,                                                                                                                 /**/
    'a', 'v', 'a', 'i', 'l', 'a', 'b', 'l', 'e', '-', 'd', 'i', 'c', 't', 'i', 'o', 'n', 'a', 'r', 'y', 0, 0, 0, 0,                                                                           /**/
    'c', 'a', 'c', 'h', 'e', '-', 'c', 'o', 'n', 't', 'r', 'o', 'l', 0, 0, 0,                                                                                                                 /**/
    'c', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', 0, 0, 0, 0, 0, 0,                                                                                                                       /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'd', 'i', 'g', 'e', 's', 't', 0, 0,                                                                                                               /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'd', 'i', 's', 'p', 'o', 's', 'i', 't', 'i', 'o', 'n', 0, 0, 0, 0, 0,                                                                             /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g',                                                                                                           /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'l', 'e', 'n', 'g', 't', 'h', 0, 0,                                                                                                               /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'l', 'o', 'c', 'a', 't', 'i', 'o', 'n',                                                                                                           /**/
    'c', 'o', 'n', 't', 'e', 'n', 't', '-', 't', 'y', 'p', 'e', 0, 0, 0, 0,                                                                                                                   /**/
    'c', 'o', 'o', 'k', 'i', 'e', 0, 0,                                                                                                                                                       /**/
    'd', 'a', 't', 'e', 0, 0, 0, 0,                                                                                                                                                           /**/
    'd', 'i', 'c', 't', 'i', 'o', 'n', 'a', 'r', 'y', '-', 'i', 'd', 0, 0, 0,                                                                                                                 /**/
    'e', 'a', 'r', 'l', 'y', '-', 'd', 'a', 't', 'a', 0, 0, 0, 0, 0, 0,                                                                                                                       /**/
    'e', 'x', 'p', 'e', 'c', 't', 0, 0,                                                                                                                                                       /**/
    'f', 'o', 'r', 'w', 'a', 'r', 'd', 'e', 'd', 0, 0, 0, 0, 0, 0, 0,                                                                                                                         /**/
    'f', 'r', 'o', 'm', 0, 0, 0, 0,                                                                                                                                                           /**/
    'i', 'd', 'e', 'm', 'p', 'o', 't', 'e', 'n', 'c', 'y', '-', 'k', 'e', 'y', 0,                                                                                                             /**/
    'i', 'f', '-', 'm', 'a', 't', 'c', 'h',                                                                                                                                                   /**/
    'i', 'f', '-', 'm', 'o', 'd', 'i', 'f', 'i', 'e', 'd', '-', 's', 'i', 'n', 'c', 'e', 0, 0, 0, 0, 0, 0, 0,                                                                                 /**/
    'i', 'f', '-', 'n', 'o', 'n', 'e', '-', 'm', 'a', 't', 'c', 'h', 0, 0, 0,                                                                                                                 /**/
    'i', 'f', '-', 'r', 'a', 'n', 'g', 'e',                                                                                                                                                   /**/
    'i', 'f', '-', 'u', 'n', 'm', 'o', 'd', 'i', 'f', 'i', 'e', 'd', '-', 's', 'i', 'n', 'c', 'e', 0, 0, 0, 0, 0,                                                                             /**/
    'k', 'e', 'e', 'p', '-', 'a', 'l', 'i', 'v', 'e', 0, 0, 0, 0, 0, 0,                                                                                                                       /**/
    'l', 'i', 'n', 'k', 0, 0, 0, 0,                                                                                                                                                           /**/
    'm', 'a', 'x', '-', 'f', 'o', 'r', 'w', 'a', 'r', 'd', 's', 0, 0, 0, 0,                                                                                                                   /**/
    'o', 'r', 'i', 'g', 'i', 'n', 0, 0,                                                                                                                                                       /**/
    'p', 'r', 'a', 'g', 'm', 'a', 0, 0,                                                                                                                                                       /**/
    'p', 'r', 'e', 'f', 'e', 'r', 0, 0,                                                                                                                                                       /**/
    'p', 'r', 'i', 'o', 'r', 'i', 't', 'y',                                                                                                                                                   /**/
    'p', 'r', 'o', 'x', 'y', '-', 'a', 'u', 't', 'h', 'o', 'r', 'i', 'z', 'a', 't', 'i', 'o', 'n', 0, 0, 0, 0, 0,                                                                             /**/
    'r', 'a', 'n', 'g', 'e', 0, 0, 0,                                                                                                                                                         /**/
    'r', 'e', 'f', 'e', 'r', 'e', 'r', 0,                                                                                                                                                     /**/
    'r', 'e', 'p', 'r', '-', 'd', 'i', 'g', 'e', 's', 't', 0, 0, 0, 0, 0,                                                                                                                     /**/
    'r', 't', 't', 0, 0, 0, 0, 0,                                                                                                                                                             /**/
    's', 'a', 'v', 'e', '-', 'd', 'a', 't', 'a', 0, 0, 0, 0, 0, 0, 0,                                                                                                                         /**/
    's', 'e', 'r', 'v', 'i', 'c', 'e', '-', 'w', 'o', 'r', 'k', 'e', 'r', 0, 0,                                                                                                               /**/
    's', 'e', 'r', 'v', 'i', 'c', 'e', '-', 'w', 'o', 'r', 'k', 'e', 'r', '-', 'n', 'a', 'v', 'i', 'g', 'a', 't', 'i', 'o', 'n', '-', 'p', 'r', 'e', 'l', 'o', 'a', 'd', 0, 0, 0, 0, 0, 0, 0, /**/
    't', 'e', 0, 0, 0, 0, 0, 0,                                                                                                                                                               /**/
    't', 'r', 'a', 'i', 'l', 'e', 'r', 0,                                                                                                                                                     /**/
    't', 'r', 'a', 'n', 's', 'f', 'e', 'r', '-', 'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g', 0, 0, 0, 0, 0, 0, 0,                                                                                 /**/
    'u', 'p', 'g', 'r', 'a', 'd', 'e', 0,                                                                                                                                                     /**/
    'u', 'p', 'g', 'r', 'a', 'd', 'e', '-', 'i', 'n', 's', 'e', 'c', 'u', 'r', 'e', '-', 'r', 'e', 'q', 'u', 'e', 's', 't', 's', 0, 0, 0, 0, 0, 0, 0,                                         /**/
    'u', 's', 'e', 'r', '-', 'a', 'g', 'e', 'n', 't', 0, 0, 0, 0, 0, 0,                                                                                                                       /**/
    'v', 'i', 'a', 0, 0, 0, 0, 0,                                                                                                                                                             /**/
    'w', 'a', 'n', 't', '-', 'c', 'o', 'n', 't', 'e', 'n', 't', '-', 'd', 'i', 'g', 'e', 's', 't', 0, 0, 0, 0, 0,                                                                             /**/
    'w', 'a', 'n', 't', '-', 'r', 'e', 'p', 'r', '-', 'd', 'i', 'g', 'e', 's', 't',                                                                                                           /**/
    'x', '-', 'f', 'o', 'r', 'w', 'a', 'r', 'd', 'e', 'd', '-', 'f', 'o', 'r', 0,                                                                                                             /**/
    'x', '-', 'f', 'o', 'r', 'w', 'a', 'r', 'd', 'e', 'd', '-', 'h', 'o', 's', 't',                                                                                                           /**/
    'x', '-', 'f', 'o', 'r', 'w', 'a', 'r', 'd', 'e', 'd', '-', 'p', 'r', 'o', 't', 'o', 0, 0, 0, 0, 0, 0, 0                                                                                  /**/
};

const uint64_t headerNamesOffsets8[34] = {
    0,                                                                                         /*size:0*/
    0,                                                                                         /*size:1*/
    ((84)),                                                                                    /*size:2|te*/
    ((74) | ((96ul) << 8)),                                                                    /*size:3|rtt,via*/
    ((37) | ((45ul) << 8) | ((60ul) << 16)),                                                   /*size:4|date,from,link*/
    ((70)),                                                                                    /*size:5|range*/
    ((0) | ((36ul) << 8) | ((42ul) << 16) | ((63ul) << 24) | ((64ul) << 32) | ((65ul) << 40)), /*size:6|accept,cookie,expect,origin,pragma,prefer*/
    ((71) | ((85ul) << 8) | ((89ul) << 16)),                                                   /*size:7|referer,trailer,upgrade*/
    ((13) | ((48ul) << 8) | ((54ul) << 16) | ((66ul) << 24)),                                  /*size:8|alt-used,if-match,if-range,priority*/
    ((43) | ((75ul) << 8)),                                                                    /*size:9|forwarded,save-data*/
    ((21) | ((40ul) << 8) | ((58ul) << 16) | ((94ul) << 24)),                                  /*size:10|connection,early-data,keep-alive,user-agent*/
    ((72)),                                                                                    /*size:11|repr-digest*/
    ((34) | ((61ul) << 8)),                                                                    /*size:12|content-type,max-forwards*/
    ((14) | ((19ul) << 8) | ((38ul) << 16) | ((52ul) << 24)),                                  /*size:13|authorization,cache-control,dictionary-id,if-none-match*/
    ((23) | ((30ul) << 8) | ((77ul) << 16)),                                                   /*size:14|content-digest,content-length,service-worker*/
    ((1) | ((3ul) << 8) | ((46ul) << 16) | ((102ul) << 24)),                                   /*size:15|accept-encoding,accept-language,idempotency-key,x-forwarded-for*/
    ((28) | ((32ul) << 8) | ((100ul) << 16) | ((104ul) << 24)),                                /*size:16|content-encoding,content-location,want-repr-digest,x-forwarded-host*/
    ((49) | ((86ul) << 8) | ((106ul) << 16)),                                                  /*size:17|if-modified-since,transfer-encoding,x-forwarded-proto*/
    0,                                                                                         /*size:18*/
    ((25) | ((55ul) << 8) | ((67ul) << 16) | ((97ul) << 24)),                                  /*size:19|content-disposition,if-unmodified-since,proxy-authorization,want-content-digest*/
    ((16)),                                                                                    /*size:20|available-dictionary*/
    0,                                                                                         /*size:21*/
    0,                                                                                         /*size:22*/
    0,                                                                                         /*size:23*/
    0,                                                                                         /*size:24*/
    ((90)),                                                                                    /*size:25|upgrade-insecure-requests*/
    0,                                                                                         /*size:26*/
    0,                                                                                         /*size:27*/
    0,                                                                                         /*size:28*/
    ((9)),                                                                                     /*size:29|access-control-request-method*/
    ((5)),                                                                                     /*size:30|access-control-request-headers*/
    0,                                                                                         /*size:31*/
    0,                                                                                         /*size:32*/
    ((79))                                                                                     /*size:33|service-worker-navigation-preload*/
};

const uint64_t headerNamesValues8[34] = {
    0,                                                                                         /*size:0*/
    0,                                                                                         /*size:1*/
    ((44)),                                                                                    /*size:2|te*/
    ((40) | ((50ul) << 8)),                                                                    /*size:3|rtt,via*/
    ((17) | ((22ul) << 8) | ((30ul) << 16)),                                                   /*size:4|date,from,link*/
    ((37)),                                                                                    /*size:5|range*/
    ((0) | ((16ul) << 8) | ((20ul) << 16) | ((32ul) << 24) | ((33ul) << 32) | ((34ul) << 40)), /*size:6|accept,cookie,expect,origin,pragma,prefer*/
    ((38) | ((45ul) << 8) | ((47ul) << 16)),                                                   /*size:7|referer,trailer,upgrade*/
    ((5) | ((24ul) << 8) | ((27ul) << 16) | ((35ul) << 24)),                                   /*size:8|alt-used,if-match,if-range,priority*/
    ((21) | ((41ul) << 8)),                                                                    /*size:9|forwarded,save-data*/
    ((9) | ((19ul) << 8) | ((29ul) << 16) | ((49ul) << 24)),                                   /*size:10|connection,early-data,keep-alive,user-agent*/
    ((39)),                                                                                    /*size:11|repr-digest*/
    ((15) | ((31ul) << 8)),                                                                    /*size:12|content-type,max-forwards*/
    ((6) | ((8ul) << 8) | ((18ul) << 16) | ((26ul) << 24)),                                    /*size:13|authorization,cache-control,dictionary-id,if-none-match*/
    ((10) | ((13ul) << 8) | ((42ul) << 16)),                                                   /*size:14|content-digest,content-length,service-worker*/
    ((1) | ((2ul) << 8) | ((23ul) << 16) | ((53ul) << 24)),                                    /*size:15|accept-encoding,accept-language,idempotency-key,x-forwarded-for*/
    ((12) | ((14ul) << 8) | ((52ul) << 16) | ((54ul) << 24)),                                  /*size:16|content-encoding,content-location,want-repr-digest,x-forwarded-host*/
    ((25) | ((46ul) << 8) | ((55ul) << 16)),                                                   /*size:17|if-modified-since,transfer-encoding,x-forwarded-proto*/
    0,                                                                                         /*size:18*/
    ((11) | ((28ul) << 8) | ((36ul) << 16) | ((51ul) << 24)),                                  /*size:19|content-disposition,if-unmodified-since,proxy-authorization,want-content-digest*/
    ((7)),                                                                                     /*size:20|available-dictionary*/
    0,                                                                                         /*size:21*/
    0,                                                                                         /*size:22*/
    0,                                                                                         /*size:23*/
    0,                                                                                         /*size:24*/
    ((48)),                                                                                    /*size:25|upgrade-insecure-requests*/
    0,                                                                                         /*size:26*/
    0,                                                                                         /*size:27*/
    0,                                                                                         /*size:28*/
    ((4)),                                                                                     /*size:29|access-control-request-method*/
    ((3)),                                                                                     /*size:30|access-control-request-headers*/
    0,                                                                                         /*size:31*/
    0,                                                                                         /*size:32*/
    ((43))                                                                                     /*size:33|service-worker-navigation-preload*/
};
#endif

//

#endif