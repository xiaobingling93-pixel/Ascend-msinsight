#!/usr/bin/env python
# -*- coding: UTF-8 -*-
# Copyright 2025 Huawei Technologies Co., Ltd
# ============================================================================

import asyncio
import logging
import unittest

loader = unittest.TestLoader()
suite = loader.discover(start_dir='test', pattern='*_test.py')
logging.disable(logging.CRITICAL)

runner = unittest.TextTestRunner(verbosity=2)

if __name__ == '__main__':
    runner.run(suite)
