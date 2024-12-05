const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();
const cmp_len: usize = 3;

fn check_for_match(wins: *const [cmp_len]*const [cmp_len]u8, ) bool {
    if (wins[1][1] != 'A') {
        return false;
    }

    if (!((wins[0][0] == 'M' and wins[2][2] == 'S') or (wins[0][0] == 'S' and wins[2][2] == 'M'))) {
        return false;
    }

    if (!((wins[2][0] == 'M' and wins[0][2] == 'S') or (wins[2][0] == 'S' and wins[0][2] == 'M'))) {
        return false;
    }

    return true;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var lines = std.ArrayList([]const u8).init(allocator);
    defer lines.deinit();

    var it = std.mem.tokenizeScalar(u8, data, '\n');
    var line_len: usize = 0;
    while (it.next()) |line| {
        if (line_len == 0) {
            line_len = line.len;
        }
        std.debug.assert(line_len >= cmp_len and line.len == line_len);

        try lines.append(line);
    }

    var result: usize = 0;
    var wins: [cmp_len]*const [cmp_len]u8 = undefined;
    var row: usize = 0;

    while (row < lines.items.len - (cmp_len - 1)) : (row += 1) {
        var col: usize = 0;

        while (col < line_len - (cmp_len - 1)) : (col += 1) {
            wins[0] = lines.items[row][col..][0..cmp_len];
            wins[1] = lines.items[row + 1][col..][0..cmp_len];
            wins[2] = lines.items[row + 2][col..][0..cmp_len];
            if (check_for_match(&wins)) {
                result += 1;
            }
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
