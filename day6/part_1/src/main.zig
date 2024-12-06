const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

const options: []const u8 = "<>^v";

fn getResult(allocator: std.mem.Allocator) !usize {
    var dir: u8 = undefined;
    var cur_line: usize = undefined;
    var cur_col: usize = undefined;

    var lines = std.ArrayList([]const u8).init(allocator);
    defer lines.deinit();

    var it = std.mem.splitScalar(u8, data, '\n');
    var row: usize = 0;
    while (it.next()) |line| : (row += 1) {
        if (line.len == 0) {
            continue;
        }

        for (options) |option| {
            const needle = [_]u8{option};
            if (std.mem.indexOf(u8, line, &needle)) |index| {
                dir = option;
                cur_line = row;
                cur_col = index;
            }
        }

        try lines.append(line);
    }

    if (dir == undefined or cur_line == undefined or cur_col == undefined) {
        return error.UnableToFindStartPos;
    }

    var visited = std.AutoHashMap(struct { usize, usize }, void).init(allocator);
    defer visited.deinit();
    var result: usize = 1;

    while (true) {
        switch (dir) {
            '^' => {
                while (cur_line >= 0) {
                    if (cur_line == 0) {
                        return result;
                    }

                    if (lines.items[cur_line - 1][cur_col] == '#') {
                        dir = '>';
                        break;
                    }

                    if (!visited.contains(.{ cur_line, cur_col })) {
                        try visited.put(.{ cur_line, cur_col }, {});
                        result += 1;
                    }

                    cur_line -= 1;
                }
            },
            '>' => {
                while (cur_col <= lines.items[cur_line].len - 1) {
                    if (cur_col + 1 == lines.items[cur_line].len) {
                        return result;
                    }

                    if (lines.items[cur_line][cur_col + 1] == '#') {
                        dir = 'v';
                        break;
                    }

                    if (!visited.contains(.{ cur_line, cur_col })) {
                        try visited.put(.{ cur_line, cur_col }, {});
                        result += 1;
                    }

                    cur_col += 1;
                }
            },
            'v' => {
                while (cur_line <= lines.items.len - 1) {
                    if (cur_line + 1 == lines.items.len) {
                        return result;
                    }

                    if (lines.items[cur_line + 1][cur_col] == '#') {
                        dir = '<';
                        break;
                    }

                    if (!visited.contains(.{ cur_line, cur_col })) {
                        try visited.put(.{ cur_line, cur_col }, {});
                        result += 1;
                    }

                    cur_line += 1;
                }
            },
            '<' => {
                while (cur_col >= 0) {
                    if (cur_col == 0) {
                        return result;
                    }

                    if (lines.items[cur_line][cur_col - 1] == '#') {
                        dir = '^';
                        break;
                    }

                    if (!visited.contains(.{ cur_line, cur_col })) {
                        try visited.put(.{ cur_line, cur_col }, {});
                        result += 1;
                    }

                    cur_col -= 1;
                }
            },
            else => unreachable,
        }
    }

    return result;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const result: usize = try getResult(allocator);

    try stdout.print("result: {d}\n", .{result});
}
