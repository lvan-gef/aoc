const std = @import("std");
const data = @embedFile("input.txt");
const stdout = std.io.getStdOut().writer();

const direction = enum { north, east, south, west };
const Guard = struct {
    dir: direction,
    row: usize,
    col: usize,
    ingrid: bool,
    inloop: bool,

    fn rotate(self: *Guard) void {
        switch (self.dir) {
            .north => self.dir = .east,
            .east => self.dir = .south,
            .south => self.dir = .west,
            .west => self.dir = .north,
        }
    }

    fn place_marker(self: *Guard, grid: *std.ArrayList(std.ArrayList([5]u8))) void {
        if (grid.items[self.row].items[self.col][0] == '-' and self.dir == .north) {
            grid.items[self.row].items[self.col][0] = '+';
            grid.items[self.row].items[self.col][1] = 1;
        } else if (grid.items[self.row].items[self.col][0] == '-' and self.dir == .south) {
            grid.items[self.row].items[self.col][0] = '+';
            grid.items[self.row].items[self.col][3] = 1;
        } else if (grid.items[self.row].items[self.col][0] == '|' and self.dir == .west) {
            grid.items[self.row].items[self.col][0] = '+';
            grid.items[self.row].items[self.col][4] = 1;
        } else if (grid.items[self.row].items[self.col][0] == '|' and self.dir == .east) {
            grid.items[self.row].items[self.col][0] = '+';
            grid.items[self.row].items[self.col][2] = 1;
        } else if (self.dir == .north) {
            grid.items[self.row].items[self.col][0] = '|';
            grid.items[self.row].items[self.col][1] = 1;
        } else if (self.dir == .south) {
            grid.items[self.row].items[self.col][0] = '|';
            grid.items[self.row].items[self.col][3] = 1;
        } else if (self.dir == .west) {
            grid.items[self.row].items[self.col][0] = '-';
            grid.items[self.row].items[self.col][4] = 1;
        } else if (self.dir == .east) {
            grid.items[self.row].items[self.col][0] = '-';
            grid.items[self.row].items[self.col][2] = 1;
        }
    }

    fn move(self: *Guard, grid: *std.ArrayList(std.ArrayList([5]u8))) void {
        if (self.row == 0 and self.dir == .north) {
            self.place_marker(grid);
            self.ingrid = false;
            return;
        }

        if (self.row == grid.items.len - 1 and self.dir == .south) {
            self.place_marker(grid);
            self.ingrid = false;
            return;
        }

        if (self.col == 0 and self.dir == .west) {
            self.place_marker(grid);
            self.ingrid = false;
            return;
        }

        if (self.col == grid.items[self.row].items.len - 1 and self.dir == .east) {
            self.place_marker(grid);
            self.ingrid = false;
            return;
        }

        var blocked = true;
        while (blocked) {
            if (self.dir == .north and grid.items[self.row - 1].items[self.col][0] == '#') {
                self.place_marker(grid);
                self.rotate();
            } else if (self.dir == .south and grid.items[self.row + 1].items[self.col][0] == '#') {
                self.place_marker(grid);
                self.rotate();
            } else if (self.dir == .west and grid.items[self.row].items[self.col - 1][0] == '#') {
                self.place_marker(grid);
                self.rotate();
            } else if (self.dir == .east and grid.items[self.row].items[self.col + 1][0] == '#') {
                self.place_marker(grid);
                self.rotate();
            } else {
                blocked = false;
            }
        }

        if ((grid.items[self.row].items[self.col][1] == 1 and self.dir == .north) or
            (grid.items[self.row].items[self.col][2] == 1 and self.dir == .east) or
            (grid.items[self.row].items[self.col][3] == 1 and self.dir == .south) or
            (grid.items[self.row].items[self.col][4] == 1 and self.dir == .west))
        {
            self.inloop = true;
            return;
        }

        self.place_marker(grid);
        if (self.dir == .north) {
            self.row -= 1;
        } else if (self.dir == .south) {
            self.row += 1;
        } else if (self.dir == .west) {
            self.col -= 1;
        } else if (self.dir == .east) {
            self.col += 1;
        }
    }
};

fn reread_grid(grid: *std.ArrayList(std.ArrayList([5]u8))) !void {
    var lines = std.mem.tokenizeScalar(u8, data, '\n');
    var irow: usize = 0;
    while (lines.next()) |line| {
        for (line, 0..) |char, icol| {
            grid.items[irow].items[icol] = [5]u8{ char, 0, 0, 0, 0 };
        }
        irow += 1;
    }
}

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var grid = std.ArrayList(std.ArrayList([5]u8)).init(allocator);
    defer _ = grid.deinit();
    defer {
        for (grid.items) |row| {
            _ = row.deinit();
        }
    }

    var lines = std.mem.tokenizeScalar(u8, data, '\n');
    while (lines.next()) |line| {
        var row = std.ArrayList([5]u8).init(allocator);
        for (line) |char| {
            try row.append([5]u8{ char, 0, 0, 0, 0 });
        }
        try grid.append(row);
    }

    var guard = Guard{
        .dir = .north,
        .row = 0,
        .col = 0,
        .ingrid = true,
        .inloop = false,
    };

    var nbarriers: i32 = 0;
    for (0..grid.items.len) |irow| {
        for (0..grid.items[irow].items.len) |icol| {
            if (grid.items[irow].items[icol][0] != '#' and grid.items[irow].items[icol][0] != '^') {
                try reread_grid(&grid);

                guard.ingrid = true;
                guard.inloop = false;
                guard.dir = .north;
                outer: for (grid.items, 0..) |row, jrow| {
                    for (row.items, 0..) |_, jcol| {
                        if (grid.items[jrow].items[jcol][0] == '^') {
                            guard.row = jrow;
                            guard.col = jcol;
                            break :outer;
                        }
                    }
                }

                grid.items[irow].items[icol][0] = '#';

                while (guard.ingrid and !guard.inloop) {
                    guard.move(&grid);
                }

                if (guard.inloop) {
                    nbarriers += 1;
                }
            }
        }
    }

    try stdout.print("result: {d}\n", .{nbarriers});
}
