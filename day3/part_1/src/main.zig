const std = @import("std");

fn getLines(allocator: std.mem.Allocator, filename: []const u8) !std.ArrayList([]u8) {
    // Open the file
    const file = try std.fs.cwd().openFile(filename, .{});
    defer file.close();

    var lines = std.ArrayList([]u8).init(allocator);
    errdefer {
        for (lines.items) |line| {
            allocator.free(line);
        }
        lines.deinit();
    }

    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    while (try in_stream.readUntilDelimiterOrEofAlloc(allocator, '\n', std.math.maxInt(usize))) |line| {
        try lines.append(line);
    }

    return lines;
}

fn get_pattern(input: []u8) u32 {
    var i: usize = 0;
    var sum: u32 = 0;

    while (i < input.len) : (i += 1) {
        if (i + 4 >= input.len) {
            break;
        }

        if (!std.mem.eql(u8, input[i..i+4], "mul(")) {
            continue;
        }

        const j = i + 4;
        if (j >= input.len) {
            break;
        }

        // Find comma position
        var comma_pos: ?usize = null;
        var k = j;
        while (k < input.len and k < j + 4) : (k += 1) {
            if (input[k] == ',') {
                comma_pos = k;
                break;
            }
        }

        if (comma_pos == null) {
            continue;
        }

        // Parse first number
        const first_num = std.fmt.parseInt(u32, input[j..comma_pos.?], 10) catch continue;

        // Find closing parenthesis
        const close_pos = std.mem.indexOfScalarPos(u8, input, comma_pos.? + 1, ')') orelse continue;
        if (close_pos >= input.len) {
            continue;
        }

        // Parse second number
        const second_num = std.fmt.parseInt(u32, input[comma_pos.? + 1..close_pos], 10) catch continue;

        sum += first_num * second_num;
        i = close_pos;
    }

    return sum;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const stdout_file = std.io.getStdOut().writer();
    var bw = std.io.bufferedWriter(stdout_file);
    const stdout = bw.writer();

    if (std.os.argv.len != 2) {
        try stdout.print("Expect 1 argument got: {d}\n", .{std.os.argv.len - 1});

        try bw.flush();
        std.process.exit(1);
    }

    var args = std.process.args();
    _ = args.skip();

    const lines = try getLines(allocator, args.next().?);
    defer lines.deinit();
    defer {
        for (lines.items) |line| {
            allocator.free(line);
        }
    }

    var result: u32 = 0;
    for (0..lines.items.len) |i| {
        result += get_pattern(lines.items[i]);
    }

    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}
