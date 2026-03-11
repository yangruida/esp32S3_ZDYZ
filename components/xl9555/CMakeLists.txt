# xl9555/CMakeLists.txt
idf_component_register(
    SRCS 
        "xl9555.c"
        "xl9555_test.c"  # 测试代码（可通过CONFIG条件编译控制是否包含）
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        driver 
        esp_io_expander 
        unity
        
)

# 可选：添加编译选项，方便调试
target_compile_definitions(${COMPONENT_LIB} PRIVATE "-DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG")