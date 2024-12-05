const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();
const cmp_len: usize = 4;
const target = "XMAS";
const rtarget = "SAMX";

fn check_for_match(wins: *const [cmp_len]*const [cmp_len]u8, all_hor: bool, all_ver: bool) !usize {
    var count: usize = 0;
    var win: [cmp_len]u8 = undefined;

    // hor
    for (0..cmp_len) |index| {
        if (std.mem.eql(u8, wins[index], target) or std.mem.eql(u8, wins[index], rtarget)) {
            count += 1;
        }

        if (!all_hor) {
            break;
        }
    }

    // vert
    for (0..cmp_len) |col_index| {
        for (0..cmp_len) |row_index| {
            win[row_index] = wins[row_index][col_index];
        }

        if (std.mem.eql(u8, &win, target) or std.mem.eql(u8, &win, rtarget)) {
            count += 1;
        }

        if (!all_ver) {
            break;
        }

    }

    // diagonal top-left to bottom-right
    for (0..cmp_len) |index| {
        win[index] = wins[index][index];
    }

    if (std.mem.eql(u8, &win, target) or std.mem.eql(u8, &win, rtarget)) {
        count += 1;
    }

    // diagonal top-right to bottom-left
    for (0..cmp_len) |index| {
        win[index] = wins[index][cmp_len - 1 - index];
    }

    if (std.mem.eql(u8, &win, target) or std.mem.eql(u8, &win, rtarget)) {
        count += 1;
    }

    return count;
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
            wins[3] = lines.items[row + 3][col..][0..cmp_len];

            const check_all_hor = row == line_len - cmp_len;
            const check_all_ver = col == line_len - cmp_len;
            result += try check_for_match(&wins, check_all_hor, check_all_ver);
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
