idf_component_register(SRCS "blink_example_main.c"
                       INCLUDE_DIRS "."
                      PRIV_REQUIRES xl9555  unity)  # 添加对 XL9555 组件的依赖)
