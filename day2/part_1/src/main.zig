const std = @import("std");

fn readNumberPairs(allocator: std.mem.Allocator, filename: []const u8) !std.ArrayList(std.ArrayList(i32)) {
    // Create the lists
    var lhs = std.ArrayList(std.ArrayList(i32)).init(allocator);
    errdefer {
        for (lhs.items) |item| {
            item.deinit();
        }
        lhs.deinit();
    }

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
        var rhs = std.ArrayList(i32).init(allocator);
        errdefer rhs.deinit();

        while (iter.next()) |part| {
            if (part.len > 0) {
                const num = try std.fmt.parseInt(i32, part, 10);
                try rhs.append(num);
            }
        }
        try lhs.append(rhs);

        line.clearRetainingCapacity();
    }

    return lhs;
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

    const lists = try readNumberPairs(allocator, args.next().?);

    var result: u32 = 0;
    for (lists.items, 0..) |v, i| {
        var found: bool = true;
        _ = i;

        // check if it is sorted
        if (!std.sort.isSorted(i32, v.items, {}, std.sort.asc(i32))) {
            if (!std.sort.isSorted(i32, v.items, {}, std.sort.desc(i32))) {
                continue;
            }
        }

        for (1..v.items.len) |index| {
            const tmp: u32 = @abs(v.items[index - 1] - v.items[index]);
            if (tmp < 1 or tmp > 3) {
                found = false;
                break;
            }
        }

        if (found) {
            result += 1;
        }
    }

    // free it
    for (lists.items) |list| {
        list.deinit();
    }
    lists.deinit();

    try stdout.print("result: {d}\n", .{result});
    try bw.flush();
}
