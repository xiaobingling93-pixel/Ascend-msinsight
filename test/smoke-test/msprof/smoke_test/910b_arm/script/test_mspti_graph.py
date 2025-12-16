import os
import glob
import pandas as pd
import test_base
import argparse
import unittest
import logging
import re
import sqlite3
from check_tools.db_check import DBManager
from collections import defaultdict

logging.basicConfig(level=logging.INFO,
                    format='\n%(asctime)s %(filename)s [line:%(lineno)d] [%(levelname)s] %(message)s')


class MsptiGraphTest(test_base.TestProfiling):
    def getTestCmd(self, scene=None):
        script_path = self.cfg_path.mspti_graph_path
        self.msprofbin_cmd += ("cd {}; source {}/set_env.sh; "
                               "source /root/miniconda3/bin/activate /root/miniconda3/envs/smoke_test_env_pta;"
                               "bash run.sh {} > {} 2>&1").format(script_path,
                                                                  self.cfg_path.ascend_toolkit_path,
                                                                  self.res_dir, self.slog_stdout)


    def check_kernel_data(self, db_path):
        logging.info(f"check_comm_data {db_path} start")
        conn, curs = DBManager.create_connect_db(db_path)
        sql = (f"SELECT kernel.*, api.* "
               f"FROM kernel "
               f"LEFT JOIN api ON kernel.correlationId = api.correlationId "
               f"WHERE api.correlationId IS NULL;")

        sql2 = ("SELECT t.* "
                "FROM kernel t "
                "JOIN ( "
                "    SELECT correlationId "
                "    FROM kernel "
                "    GROUP BY correlationId "
                "    HAVING COUNT(*) NOT IN (1, 3) "
                ") bad ON t.correlationId = bad.correlationId;")
        try:
            curs.execute(sql)
            rows = curs.fetchall()
            if rows:
                for r in rows:
                    logging.info(f"Unexpected data: {r}")
            assert not rows, "Found unexpected data in kernel check"

            curs.execute(sql2)
            rows = curs.fetchall()
            if rows:
                for r in rows:
                    logging.info(f"Unexpected data: {r}")
            assert not rows, "Found unexpected graph data in kernel check"
        except sqlite3.Error as _err:
            assert False, f"Failed to fetch data, ERROR: {_err}, dataPath: {db_path}"
        finally:
            DBManager.destroy_db_connect(conn, curs)

    def check_api_data(self, db_path):
        logging.info(f"check_api_data {db_path} start")
        conn, curs = DBManager.create_connect_db(db_path)
        hccl_api_name = "hcom_%"
        sql = (f"SELECT kernel.*, api.* "
               f"FROM api "
               f"LEFT JOIN kernel ON kernel.correlationId = api.correlationId "
               f"WHERE kernel.correlationId IS NULL and api.name not like '{hccl_api_name}' ;")
        try:
            curs.execute(sql)
            rows = curs.fetchall()
            if rows:
                for r in rows:
                    logging.info(f"Unexpected data: {r}")
            assert not rows, "Found unexpected data in api check"
        except sqlite3.Error as _err:
            assert False, f"Failed to fetch data, ERROR: {_err}, dataPath: {db_path}"
        finally:
            DBManager.destroy_db_connect(conn, curs)

    def check_comm_data(self, db_path):
        logging.info(f"check_correlation {db_path} start")
        conn, curs = DBManager.create_connect_db(db_path)
        sql = (f"SELECT communication.name, api.name "
               f"FROM communication "
               f"LEFT JOIN api ON communication.correlationId = api.correlationId "
               f"where communication.name != api.name;")
        try:
            curs.execute(sql)
            rows = curs.fetchall()
            if rows:
                for r in rows:
                    logging.info(f"Unexpected data: {r}")
            assert not rows, "Found unexpected data in comm check"
        except sqlite3.Error as _err:
            assert False, f"Failed to fetch data, ERROR: {_err}, dataPath: {db_path}"
        finally:
            DBManager.destroy_db_connect(conn, curs)

    def checkResDir(self, scend=None):
        db_paths = glob.glob(os.path.join(self.res_dir, "activity_log_*.db"))
        for db_path in db_paths:
            self.check_comm_data(db_path)
            self.check_api_data(db_path)
            self.check_kernel_data(db_path)


if __name__ == '__main__':
    suite = unittest.TestSuite()
    suite.addTest(
        MsptiGraphTest("test_mspti_graph", "mspti_c", "", "", timeout=480))
    unittest.TextTestRunner(verbosity=2).run(suite)
