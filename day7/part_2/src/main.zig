const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

const Operator = enum {
    add,
    multiply,
    concatenate,

    pub fn apply(self: Operator, a: usize, b: usize) usize {
        return switch (self) {
            .add => a + b,
            .multiply => a * b,
            .concatenate => {
                var b_buf: [20]u8 = undefined;
                const b_str = std.fmt.bufPrint(&b_buf, "{d}", .{b}) catch return 0;
                const multiplier = std.math.pow(usize, 10, b_str.len);
                return (a * multiplier) + b;
            },
        };
    }
};

fn findSolution(allocator: std.mem.Allocator, target: usize, numbers: []const usize) !bool {
    const num_operators = numbers.len - 1;

    var operators = try allocator.alloc(Operator, num_operators);
    defer allocator.free(operators);

    const max_combinations = std.math.pow(usize, 3, num_operators);
    var combination: usize = 0;

    while (combination < max_combinations) : (combination += 1) {
        var temp = combination;
        for (operators, 0..) |_, i| {
            const op_value = temp % 3;
            operators[i] = switch (op_value) {
                0 => .add,
                1 => .multiply,
                2 => .concatenate,
                else => unreachable,
            };
            temp /= 3;
        }

        var result = numbers[0];
        for (operators, 0..) |op, i| {
            result = op.apply(result, numbers[i + 1]);
        }

        if (result == target) {
            return true;
        }

        const temp_result = operators[operators.len - 1].apply(numbers[numbers.len - 2], numbers[numbers.len - 1]);
        const final_result = operators[0].apply(numbers[0], temp_result);

        if (final_result == target) {
            return true;
        }
    }

    return false;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var inputs = std.ArrayList(usize).init(allocator);
    defer inputs.deinit();

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

        if (try findSolution(allocator, target, inputs.items)) {
            result += target;
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
