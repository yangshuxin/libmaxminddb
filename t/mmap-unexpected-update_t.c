#include "maxminddb_test_helper.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
static int cp(const char * from, const char * to)
{
    ssize_t read_bytes;
    char buf[1024];
    int f, t, rc = -1, bsize = sizeof(buf);
    if ((f = open(from, O_RDONLY)) >= 0) {
        if ((t = open(to, O_WRONLY | O_CREAT, 0666)) >= 0) {
            rc = 0;
            while ((read_bytes = read(f, buf, bsize)) > 0) {
                if (write(t, buf, read_bytes) != read_bytes) {
                    rc = -1;
                    break;
                }
            }
            close(t);
        }
        close(f);
    }
    return rc;
}

#define TMP_DIR "/tmp"

void run_tests(int mode, const char *mode_desc)
{
    char *dest_large, *dest_small, *dest_db;
    const char *filename_large = "GeoIP2-City-Test.mmdb";
    const char *filename_small = "MaxMind-DB-test-mixed-24.mmdb";
    const char *path_large = test_database_path(filename_large);
    const char *path_small = test_database_path(filename_small);

    asprintf(&dest_large, "%s/%s", TMP_DIR, filename_large);
    asprintf(&dest_small, "%s/%s", TMP_DIR, filename_small);
    asprintf(&dest_db, "%s/%s", TMP_DIR, "GeoIP2-test-db.mmdb");
    int err = cp(path_large, dest_db);
    cmp_ok(err, "==", 0, "Copy database file sucessfull");

    MMDB_s *mmdb = open_ok(dest_db, mode, mode_desc);

    const char *ip = "::202.196.224.0";
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result =
        MMDB_lookup_string(mmdb, ip, &gai_error, &mmdb_error);

    cmp_ok(
        mmdb_error, "==", MMDB_SUCCESS,
        "MMDB_lookup_string sets mmdb_error to MMDB_SUCCESS");

     err = cp(path_small, dest_small);
     cmp_ok(err, "==", 0, "Copy database file sucessfull");
    rename(dest_small, dest_db);
    sleep(2);

    result =
        MMDB_lookup_string(mmdb, ip, &gai_error, &mmdb_error);

    cmp_ok(
        mmdb_error, "==", MMDB_SUCCESS,
        "MMDB_lookup_string sets mmdb_error to MMDB_SUCCESS");

    free(dest_large);
    free(dest_small);
    free(dest_db);

    MMDB_close(mmdb);
    free(mmdb);
}

int main(void)
{
    plan(NO_PLAN);
    for_all_modes(&run_tests);
    done_testing();
}
