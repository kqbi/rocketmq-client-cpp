/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "Logging.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include "UtilAll.h"
#define BOOST_DATE_TIME_SOURCE

namespace rocketmq {
logAdapter* logAdapter::alogInstance;
boost::mutex logAdapter::m_imtx;

logAdapter::~logAdapter() {
  //only remove current sink
  logging::core::get()->remove_sik(m_logSink);
  m_logSink->stop();
  m_logSink->flush();
}

logAdapter* logAdapter::getLogInstance() {
  if (alogInstance == NULL) {
    boost::mutex::scoped_lock guard(m_imtx);
    if (alogInstance == NULL) {
      alogInstance = new logAdapter();
    }
  }
  return alogInstance;
}

logAdapter::logAdapter() : m_logLevel(eLOG_LEVEL_INFO) {
  setLogDir();
  //use current dir
  string homeDir;
  homeDir.append(m_log_dir);
  m_logFile += homeDir;
  std::string fileName = "rocketmq_client.log";
  m_logFile += fileName;

  // boost::log::expressions::attr<
  // boost::log::attributes::current_thread_id::value_type>("ThreadID");
  boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
  m_logSink = logging::add_file_log(keywords::file_name = m_logFile,
                                    keywords::target_file_name = "rocketmq_client_%Y%m%d-%N.log",
                                    keywords::rotation_size = 100 * 1024 * 1024,
                                    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
                                    keywords::format = "[%TimeStamp%](%Severity%):%Message%",
                                    keywords::min_free_space = 300 * 1024 * 1024, keywords::target = homeDir,
                                    keywords::max_size = 200 * 1024 * 1024,  // max keep 3 log file defaultly
                                    keywords::auto_flush = true);
  // logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
  setLogLevelInner(m_logLevel);

  logging::add_common_attributes();
}

void logAdapter::setLogLevelInner(elogLevel logLevel) {
  switch (logLevel) {
    case eLOG_LEVEL_FATAL:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::fatal);
      break;
    case eLOG_LEVEL_ERROR:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::error);
      break;
    case eLOG_LEVEL_WARN:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::warning);
      break;
    case eLOG_LEVEL_INFO:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::info);
      break;
    case eLOG_LEVEL_DEBUG:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::debug);
      break;
    case eLOG_LEVEL_TRACE:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::trace);
      break;
    default:
      //use current sink
      m_logSink->set_filter(logging::trivial::severity >= logging::trivial::info);
      break;
  }
}
void logAdapter::setLogLevel(elogLevel logLevel) {
  m_logLevel = logLevel;
  setLogLevelInner(logLevel);
}

elogLevel logAdapter::getLogLevel() {
  return m_logLevel;
}

void logAdapter::setLogDir() {
  char* p = nullptr;
  if ((p = getenv(ROCKETMQ_CLIENT_LOG_DIR.c_str()))) {
    m_log_dir = p;
  }
  if (!m_log_dir.empty()) {
    if (m_log_dir[m_log_dir.length() - 1] != '/') {
      m_log_dir += '/';
    }
  } else {
    m_log_dir = "/logs/rocketmq-client/";
  }
}

void logAdapter::setLogFileNumAndSize(int logNum, int sizeOfPerFile) {
  //use current dir
  string homeDir;
  homeDir.append(m_log_dir);
  m_logSink->locked_backend()->set_file_collector(sinks::file::make_collector(
      keywords::target = homeDir, keywords::max_size = logNum * sizeOfPerFile * 1024 * 1024));
}
}  // namespace rocketmq
