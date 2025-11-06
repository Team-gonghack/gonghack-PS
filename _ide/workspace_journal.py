# 2025-11-06T13:59:55.696559700
import vitis

client = vitis.create_client()
client.set_workspace(path="gonghack-PS")

platform = client.get_component(name="gonghack_pl")
status = platform.build()

status = platform.build()

comp = client.get_component(name="gonghack_PS")
comp.build()

status = platform.build()

comp.build()

status = platform.build()

status = platform.build()

comp.build()

vitis.dispose()

