import homepage from "./index.html";

const server = Bun.serve({
  port: Number(Bun.env.PORT ?? 3000),
  development: Bun.env.NODE_ENV !== "production",
  routes: {
    "/": homepage,
    "/api/hello": {
      GET: () =>
        Response.json({
          message: "Hello from Bun!",
          timestamp: new Date().toISOString(),
        }),
    },
  },
  fetch(request, server) {
    if (new URL(request.url).pathname === "/ws") {
      if (server.upgrade(request)) return;

      return new Response("WebSocket upgrade failed", { status: 400 });
    }

    return Response.json({ error: "Not found" }, { status: 404 });
  },
  websocket: {
    open(socket) {
      socket.send("Connected to the WebSocket server");
    },
    message(socket, message) {
      socket.send(message);
    },
    close() {
      console.log("WebSocket client disconnected");
    },
  },
});

console.log(`Server running at ${server.url}`);
