FROM alpine:latest AS build

RUN apk add --no-cache gcc musl-dev upx

WORKDIR /app

COPY server.c .
COPY server-healthcheck.c .
COPY public/ public/

RUN gcc server.c -static -Os -o server && \
    gcc server-healthcheck.c -static -Os -o server-healthcheck && \
    strip server server-healthcheck && \
    upx --best --lzma server server-healthcheck

FROM scratch

WORKDIR /app

COPY --from=build /app/server .
COPY --from=build /app/server-healthcheck .

COPY --from=build /app/public/ public/

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
  CMD ["./server-healthcheck"]

CMD ["./server"]
