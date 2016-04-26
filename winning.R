library(pracma)
library(stringi)

board <- function(i, n) {
    x <- strsplit(stri_pad_left(bits(i), n^2, "0"), "")[[1]]
    matrix(as.numeric(x), n, n)
}

fliph <- function(x) {
    t(apply(x, 1, rev))
}

flipv <- function(x) {
    apply(x, 2, rev)
}

rot90 <- function(x) {
    t(fliph(x))
}

rot180 <- function(x) {
    fliph(flipv(x))
}

rot270 <- function(x) {
    t(flipv(x))
}

funs <- list(identity, rot90, rot180, rot270, t, fliph, function(x) t(rot180(x)), flipv)

isuniq <- function(x) {
    lapply(funs[-1], do.call, list(x))
}

vert <- as.numeric(readLines("list"))

foo <- lapply(vert, board, 5)

i <- 1
while (i < length(foo)) {
    x <- na.omit(unlist(lapply(isuniq(foo[[i]]), function(x) which(sapply(foo, identical, x)))))
    x <- x[x != i]
    if (length(x) >= 1)
        foo <- foo[-x]
    i <- i + 1
}

bar <- foo

bin <- function(x) {
    strtoi(paste0(rev(as.character(x)), collapse = ""), base = 2)
}

bar <- lapply(foo, function(x) {
    enum <- lapply(funs, do.call, list(x))
    enum[[which.min(sapply(enum, bin))]]
})

unique(lapply(bar, function(x) x[1:4, 1:4]))
