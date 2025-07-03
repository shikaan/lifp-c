const std = @import("std");
const lexer = @import("lexer.zig");

pub fn main() !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();

    const stdin = std.io.getStdIn();
    const buffer = try stdin.readToEndAlloc(allocator, 1024 * 16);

    var lex = lexer.Lexer{ .allocator = allocator };

    const tokens = lex.tokenize(buffer) catch |err| switch (err) {
        error.UnexpectedToken => {
            std.log.err("Unexpected token '{s}' at {}:{}", .{ lex.token, lex.position.line, lex.position.column });
            return err;
        },
        else => return err,
    };

    for (tokens.items) |tok| {
        std.log.debug("{}", .{tok});
    }
}
