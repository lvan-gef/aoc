const std = @import("std");
const data = @embedFile("input.txt");

fn mul(win: []const u8) !usize {
    const slice1 = std.mem.sliceTo(win[4..], ',');
    const lhs = try std.fmt.parseInt(usize, slice1, 0);

    const slice2 = std.mem.sliceTo(win[5 + slice1.len ..], ')');
    const rhs = try std.fmt.parseInt(usize, slice2, 0);

    return lhs * rhs;
}

fn get_result() usize {
    var active: bool = true;
    var total: usize = 0;
    var it = std.mem.window(u8, data, 12, 1);

    while (it.next()) |win| {
        if (std.mem.eql(u8, win[0..4], "do()")) {
            active = true;
        }

        if (std.mem.eql(u8, win[0..7], "don't()")) {
            active = false;
        }

        if (std.mem.eql(u8, win[0..4], "mul(")) {
            if (active) {
                total += mul(win) catch continue;
            }
        }
    }

    return total;
}

pub fn main() !void {
    const stdout_file = std.io.getStdOut().writer();
    var bw = std.io.bufferedWriter(stdout_file);
    const stdout = bw.writer();

    const result = get_result();
    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}
