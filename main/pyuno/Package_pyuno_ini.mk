#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Package_Package,pyuno_pyuno_ini,$(SRCDIR)/pyuno))
ifeq ($(GUI),UNX)
$(eval $(call gb_Package_add_file,pyuno_pyuno_ini,lib/pyunorc,source/module/pyuno))
else ifeq ($(GUI),OS2)
$(eval $(call gb_Package_add_file,pyuno_pyuno_ini,bin/pyuno.ini,source/module/pyuno))
else
$(eval $(call gb_Package_add_file,pyuno_pyuno_ini,bin/pyuno.ini,source/module/pyuno))
endif
