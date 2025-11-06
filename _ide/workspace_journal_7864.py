# 2025-11-06T13:28:39.864767600
import vitis

client = vitis.create_client()
client.set_workspace(path="gonghack-PS")

platform = client.get_component(name="gonghack_pl")
status = platform.build()

status = platform.build()

vitis.dispose()

