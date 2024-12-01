const std = @import("std");

const NumberLists = struct {
    lhs: std.ArrayList(u32),
    rhs: std.ArrayList(u32),
};

fn compare(a: u32, b: u32) bool {
    return a < b;
}

fn readNumberPairs(allocator: std.mem.Allocator, filename: []const u8) !NumberLists {
    // Create the lists
    var lhs = std.ArrayList(u32).init(allocator);
    errdefer lhs.deinit();
    var rhs = std.ArrayList(u32).init(allocator);
    errdefer rhs.deinit();

    // Open the file
    const file = try std.fs.cwd().openFile(filename, .{});
    defer file.close();

    // Create a buffered reader
    var buf_reader = std.io.bufferedReader(file.reader());
    var in_stream = buf_reader.reader();

    var line = std.ArrayList(u8).init(allocator);
    defer line.deinit();

    while (true) {
        in_stream.readUntilDelimiterArrayList(&line, '\n', 1024) catch |err| switch (err) {
            error.EndOfStream => break,
            else => return err,
        };

        var iter = std.mem.splitScalar(u8, line.items, ' ');

        if (iter.next()) |part| {
            const num = try std.fmt.parseInt(u32, part, 10);
            try lhs.append(num);
        }

        while (iter.next()) |part| {
            if (part.len > 0) {
                const num = try std.fmt.parseInt(u32, part, 10);
                try rhs.append(num);
                break;
            }
        }

        line.clearRetainingCapacity();
    }

    return NumberLists{ .lhs = lhs, .rhs = rhs };
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

    var lists = try readNumberPairs(allocator, args.next().?);
    defer lists.lhs.deinit();
    defer lists.rhs.deinit();

    var result: u32 = 0;
    for (lists.lhs.items, 0..) |value, i| {
        _ = i;

        var count: u32 = 0;
        for (lists.rhs.items) |x| {
            if (x == value) {
                count += 1;
            }
        }
        // const count = std.mem.count(lists.rhs.items, |x| x == value);
        result += (value * count);
    }

    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}

