const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

fn get_numbers(allocator: std.mem.Allocator) !std.ArrayList(std.ArrayList(i32)) {
    var list = std.ArrayList(std.ArrayList(i32)).init(allocator);
    errdefer {
        for (list.items) |item| {
            item.deinit();
        }
        list.deinit();
    }

    var it = std.mem.tokenizeScalar(u8, data, '\n');
    while (it.next()) |token| {
        var sub_list = std.ArrayList(i32).init(allocator);
        errdefer sub_list.deinit();

        var values = std.mem.tokenizeScalar(u8, token, ' ');
        while (values.next()) |value| {
            const int_value = try std.fmt.parseInt(i32, value, 10);
            try sub_list.append(int_value);
        }

        try list.append(sub_list);
    }

    return list;
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const lists = try get_numbers(allocator);
    defer lists.deinit();
    defer {
        for (lists.items) |list| {
        list.deinit();
        }
    }

    var result: u32 = 0;
    for (lists.items) |value| {
        var found: bool = true;

        // check if it is sorted
        if (!std.sort.isSorted(i32, value.items, {}, std.sort.asc(i32))) {
            if (!std.sort.isSorted(i32, value.items, {}, std.sort.desc(i32))) {
                continue;
            }
        }

        for (1..value.items.len) |index| {
            const tmp: u32 = @abs(value.items[index - 1] - value.items[index]);
            if (tmp < 1 or tmp > 3) {
                found = false;
                break;
            }
        }

        if (found) {
            result += 1;
        }
    }

    try stdout.print("result: {d}\n", .{result});
}
