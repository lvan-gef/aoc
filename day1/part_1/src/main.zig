const std = @import("std");
const data = @embedFile("input.txt");

fn get_result(allocator: std.mem.Allocator) !usize {
    var lhs = std.ArrayList(i32).init(allocator);
    defer lhs.deinit();

    var rhs = std.ArrayList(i32).init(allocator);
    defer rhs.deinit();

    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |token| {
        var x = std.mem.tokenizeScalar(u8, token, ' ');
        const lhs_val = x.next().?;
        const rhs_val = x.next().?;
        try lhs.append(try std.fmt.parseInt(i32, lhs_val, 10));
        try rhs.append(try std.fmt.parseInt(i32, rhs_val, 10));
    }

    std.mem.sort(i32, lhs.items, {}, std.sort.asc(i32));
    std.mem.sort(i32, rhs.items, {}, std.sort.asc(i32));

    var result: usize = 0;
    for (lhs.items, 0..) |value, i| {
        result += @abs(value - rhs.items[i]);
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
