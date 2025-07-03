const std = @import("std");

pub const TokenType = enum {
    integer,
    symbol,
    lparen,
    rparen,
};

pub const BufferPosition = struct {
    line: usize,
    column: usize,
};

pub const TokenValue = union(TokenType) {
    integer: i32,
    symbol: []const u8,
    lparen: void,
    rparen: void,
};

pub const Token = struct { value: TokenValue, position: BufferPosition };

pub const Lexer = struct {
    position: BufferPosition = .{ .line = 0, .column = 0 },
    token: [64]u8 = undefined,
    allocator: std.mem.Allocator,

    pub fn tokenize(self: *Lexer, buffer: []const u8) !std.ArrayList(Token) {
        var list = std.ArrayList(Token).init(self.allocator);
        self.position.line = 1;
        self.position.column = 0;

        for (buffer) |char| {
            self.position.column += 1;
            self.token[0] = char;

            const token = try switch (char) {
                '(' => Token{ .value = TokenValue{ .lparen = {} }, .position = self.position },
                ')' => Token{ .value = TokenValue{ .rparen = {} }, .position = self.position },
                '0'...'9' => Token{ .value = TokenValue{ .integer = char - '0' }, .position = self.position },
                ' ', '\t', '\r', std.ascii.control_code.vt, std.ascii.control_code.ff => continue,
                '\n' => {
                    self.position.line += 1;
                    self.position.column = 0;
                    continue;
                },
                else => error.UnexpectedToken,
            };

            try list.append(token);
        }

        return list;
    }
};

test "atoms" {
    const TestCase = struct {
        input: []const u8,
        expected: []const Token,
    };

    const tests = [_]TestCase{
        .{
            .input = "1",
            .expected = &.{
                Token{ .value = .{ .integer = 1 }, .position = .{ .line = 1, .column = 1 } },
            },
        },
        .{
            .input = "(",
            .expected = &.{
                Token{ .value = .{ .lparen = {} }, .position = .{ .line = 1, .column = 1 } },
            },
        },
        .{
            .input = ")",
            .expected = &.{
                Token{ .value = .{ .rparen = {} }, .position = .{ .line = 1, .column = 1 } },
            },
        },
        .{
            .input = "13",
            .expected = &.{
                Token{ .value = .{ .integer = 1 }, .position = .{ .line = 1, .column = 1 } },
                Token{ .value = .{ .integer = 3 }, .position = .{ .line = 1, .column = 2 } },
            },
        },
    };

    var lexer = Lexer{ .allocator = std.testing.allocator };
    for (tests) |case| {
        const result = try lexer.tokenize(case.input);
        defer result.deinit();
        try std.testing.expectEqualDeep(result.items, case.expected);
    }
}

test "whitespaces" {
    const TestCase = struct {
        input: []const u8,
        expected: []const Token,
    };

    const tests = [_]TestCase{
        .{
            .input = "(\t1\n)",
            .expected = &.{
                Token{ .value = .{ .lparen = {} }, .position = .{ .line = 1, .column = 1 } },
                Token{ .value = .{ .integer = 1 }, .position = .{ .line = 1, .column = 3 } },
                Token{ .value = .{ .rparen = {} }, .position = .{ .line = 2, .column = 1 } },
            },
        },
        .{
            .input = "( 1\r)",
            .expected = &.{
                Token{ .value = .{ .lparen = {} }, .position = .{ .line = 1, .column = 1 } },
                Token{ .value = .{ .integer = 1 }, .position = .{ .line = 1, .column = 3 } },
                Token{ .value = .{ .rparen = {} }, .position = .{ .line = 1, .column = 5 } },
            },
        },
    };

    var lexer = Lexer{ .allocator = std.testing.allocator };
    for (tests) |case| {
        const result = try lexer.tokenize(case.input);
        defer result.deinit();
        try std.testing.expectEqualDeep(case.expected, result.items);
    }
}
