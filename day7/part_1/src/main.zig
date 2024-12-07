const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

const Operator = enum {
    add,
    multiply,

    pub fn apply(self: Operator, a: usize, b: usize) usize {
        return switch (self) {
            .add => a + b,
            .multiply => a * b,
        };
    }
};

fn findSolution(allocator: std.mem.Allocator, target: usize, numbers: []const usize) !bool {
    const num_operators = numbers.len - 1;

    var operators = try allocator.alloc(Operator, num_operators);
    defer allocator.free(operators);

    const max_combinations = std.math.pow(usize, 2, num_operators);
    var combination: usize = 0;

    while (combination < max_combinations) : (combination += 1) {
        var temp = combination;
        for (operators, 0..) |_, i| {
            operators[i] = if (temp & 1 == 0) .add else .multiply;
            temp >>= 1;
        }

        var result = numbers[0];
        for (operators, 0..) |op, i| {
            result = op.apply(result, numbers[i + 1]);
        }

        if (result == target) {
            return true;
        }
    }

    return false;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var result: usize = 0;
    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |line| {
        var tokens = std.mem.tokenizeScalar(u8, line, ' ');
        const nbr = tokens.next().?;
        const target = try std.fmt.parseInt(usize, nbr[0 .. nbr.len - 1], 10);

        var inputs = std.ArrayList(usize).init(allocator);
        defer inputs.deinit();

        while (tokens.next()) |token| {
            try inputs.append(try std.fmt.parseInt(usize, token, 10));
        }

        if (try findSolution(allocator, target, inputs.items)) {
            result += target;
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
