# 2025-11-06T11:25:09.734441600
import vitis

client = vitis.create_client()
client.set_workspace(path="gonghack-PS")

advanced_options = client.create_advanced_options_dict(dt_overlay="0")

platform = client.create_platform_component(name = "gonghack_pl",hw_design = "$COMPONENT_LOCATION/../../Care-Spine_HDL/Care-Spine/design_1_wrapper.xsa",os = "standalone",cpu = "ps7_cortexa9_0",domain_name = "standalone_ps7_cortexa9_0",generate_dtb = False,advanced_options = advanced_options,compiler = "gcc")

platform = client.get_component(name="gonghack_pl")
status = platform.build()

comp = client.create_app_component(name="gonghack_PS",platform = "$COMPONENT_LOCATION/../gonghack_pl/export/gonghack_pl/gonghack_pl.xpfm",domain = "standalone_ps7_cortexa9_0")

status = platform.build()

comp = client.get_component(name="gonghack_PS")
comp.build()

status = comp.clean()

status = platform.build()

comp.build()

vitis.dispose()

vitis.dispose()

