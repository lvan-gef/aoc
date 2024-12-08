const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

const MAX_NUMBERS = 12;
const MAX_OPERATIONS = MAX_NUMBERS - 1;

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

fn findSolution(operators: []Operator, target: usize, numbers: []const usize) bool {
    const num_operators = numbers.len - 1;
    const max_combinations = std.math.pow(usize, 2, num_operators);
    var combination: usize = 0;

    while (combination < max_combinations) : (combination += 1) {
        const temp = combination;
        for (operators[0..num_operators]) |*op| {
            op.* = @enumFromInt(temp % 2);
        }

        var result = numbers[0];
        for (operators[0..num_operators], 0..) |op, i| {
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

    var inputs = try std.ArrayList(usize).initCapacity(allocator, MAX_NUMBERS);
    defer inputs.deinit();

    var operations: [MAX_OPERATIONS]Operator = undefined;

    var result: usize = 0;
    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |line| {
        inputs.clearRetainingCapacity();

        var tokens = std.mem.tokenizeScalar(u8, line, ' ');
        const nbr = tokens.next().?;
        const target = try std.fmt.parseInt(usize, nbr[0 .. nbr.len - 1], 10);

        while (tokens.next()) |token| {
            try inputs.append(try std.fmt.parseInt(usize, token, 10));
        }

        std.debug.assert(inputs.items.len <= MAX_NUMBERS);
        if (findSolution(&operations, target, inputs.items)) {
            result += target;
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
