const std = @import("std");
const data = @embedFile("input.txt");

fn get_result(allocator: std.mem.Allocator) !usize {
    var lhs = std.ArrayList(usize).init(allocator);
    defer lhs.deinit();

    var rhs = std.ArrayList(usize).init(allocator);
    defer rhs.deinit();

    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |token| {
        var values = std.mem.tokenizeScalar(u8, token, ' ');
        const lhs_val = values.next().?;
        const rhs_val = values.next().?;
        try lhs.append(try std.fmt.parseInt(usize, lhs_val, 10));
        try rhs.append(try std.fmt.parseInt(usize, rhs_val, 10));
    }

    var result: usize = 0;
    for (lhs.items) |l_value| {
        var count: usize = 0;
        for (rhs.items) |r_value| {
            if (r_value == l_value) {
                count += 1;
            }
        }

        result += (l_value * count);
    }

    return result;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const stdout_file = std.io.getStdOut().writer();
    var bw = std.io.bufferedWriter(stdout_file);
    const stdout = bw.writer();

    const result = try get_result(allocator);

    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}
